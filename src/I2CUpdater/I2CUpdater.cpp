#include "I2CUpdater.hpp"

namespace SDUpdaterNS
{
  namespace I2CUpdater
  {

    TwoWire* bus = nullptr;
    Stream* stream = nullptr;

    constexpr const uint8_t REG_MAGIC_BYTE       = 0xaa;
    constexpr const uint8_t REG_CMD_START_UPDATE = 0x02;
    constexpr const uint8_t REG_CMD_READ_DATA    = 0x03;
    constexpr const uint8_t ok_message[2] = {'o','k'};
    constexpr const uint8_t ko_message[2] = {'o','k'};
    constexpr const size_t buffer_size = 64;

    static uint8_t buf[buffer_size] = {0};
    static uint8_t I2C_UPDATER_ADDR = 0x55;
    static size_t update_size = 0;
    static bool update_pending = false;


    inline onProgress_t on_progress = [](size_t progress, size_t total )
    {
      static size_t last = -1;
      if( last != progress ) {
        log_d("%d/%d", progress, total );
        last = progress;
      }
    };


    void onProgress( onProgress_t cb )
    {
      on_progress = cb;
    }


    namespace Bus
    {

      size_t sendData( uint8_t reg, uint8_t* data=nullptr, size_t data_size=0 )
      {
        assert( bus );
        uint8_t ret = 0;
        bus->beginTransmission( I2C_UPDATER_ADDR );
        ret += bus->write( REG_MAGIC_BYTE );
        ret += bus->write( reg );
        ret += bus->write( (uint8_t*)&data_size, sizeof(size_t) );
        if( data_size>0 ) {
          ret += bus->write( data, data_size );
        }
        bus->endTransmission(true); // end
        return ret;
      }


      size_t sendReg( uint8_t reg, uint8_t* data=nullptr, size_t size=0 )
      {
        assert( bus );
        uint8_t ret = 0;
        bus->beginTransmission( I2C_UPDATER_ADDR );
        bus->write( REG_MAGIC_BYTE );
        bus->write( reg );
        if( size>0 ) {
          bus->write( data, size );
        }
        ret = bus->endTransmission(true); // end
        return ret;
      }


      size_t readReg( int8_t reg, uint8_t* args, size_t args_size, uint8_t *buf, size_t bufsize )
      {
        assert( bus );
        uint8_t ret = sendReg( reg, args, args_size );
        ret = bus->requestFrom( (I2C_UPDATER_ADDR), bufsize );
        if( ret == bufsize ) {
          bus->readBytes( buf, bufsize );
        }
        return ret;
      }

    }; // end namespace Bus



    namespace Control // A.K.A. server, master, the ESP serving the flash data
    {

      void setup( TwoWire* srcBus, uint8_t dev_addr, Stream *srcStream, size_t updateSize )
      {
        assert( srcBus );
        assert( srcStream );
        assert( updateSize>0 );
        I2CUpdater::stream           = srcStream;
        I2CUpdater::bus              = srcBus;
        I2CUpdater::update_size      = updateSize;
        I2CUpdater::I2C_UPDATER_ADDR = dev_addr;
      }


      void flashI2C() // Called from Update-source master I2C when Update-target is slave I2C
      {
        uint8_t response[2];
        // send update signal to the slave
        Bus::readReg( REG_CMD_START_UPDATE, (uint8_t*)&update_size, sizeof(size_t), response, 2 );
        if( response[0] != 'o' || response[1] != 'k' ) {
          log_e("REG_CMD_START_UPDATE failed: 0x%02x 0x%02x ", response[0], response[1]);
          return;
        }

        size_t sent_bytes = 0;

        do {
          size_t bytes_read = stream->readBytes( buf, buffer_size );
          if( bytes_read ) {
            //log_d("Writing %d bytes to master", bytes_read );
            if( Bus::sendData( REG_CMD_READ_DATA, buf, bytes_read ) ) {
              sent_bytes += bytes_read;

              float fprogress = (float(sent_bytes)/float(update_size))*100.0;
              if( I2CUpdater::on_progress ) I2CUpdater::on_progress( fprogress, 100 );

              if( sent_bytes < update_size ) {
                bus->requestFrom( (I2C_UPDATER_ADDR), (uint8_t)1 );
                bus->read();
              }

            } else {
              log_e("couldn't send %d bytes from stream", buffer_size );
              log_print_buf(buf, bytes_read);
              break;
            }
          } else {
            log_e("couldn't read %d bytes from stream", buffer_size );
            log_print_buf(buf, buffer_size);
            break;
          }
        } while( sent_bytes < update_size );

        log_d("Sent %d bytes", sent_bytes);
      }

    }; // end namespace Control



    namespace Target // A.K.A. client, slave, the ESP to be flashed
    {

      void setup( TwoWire* srcBus, uint8_t dev_addr )
      {
        assert( srcBus );
        I2CUpdater::bus              = srcBus;
        I2CUpdater::I2C_UPDATER_ADDR = dev_addr;
        I2CUpdater::update_size      = 0;
        I2CUpdater::update_pending   = false;
        bus->onReceive(Target::onReceive);
      }


      void onReceive( int len ) // Flashed agent acting as a slave
      {
        assert( bus );
        if( !bus->available() ) {
          log_e("Bus is not available");
          return;
        }
        // read all bytes as early as possible to prevent fifo from being full
        uint8_t rcv_buf[len];
        bus->readBytes( rcv_buf, len );

        int offset = 0;

        uint8_t magic_byte = rcv_buf[offset++];// first byte is magic byte

        if( len==1 || magic_byte != REG_MAGIC_BYTE ) { // junk request
          return;
        }

        uint8_t reg_cmd = rcv_buf[offset++];// second byte is command

        static size_t processed_bytes = 0;

        switch( reg_cmd ) {
          case REG_CMD_START_UPDATE:
          {
            update_size = *((size_t*)&rcv_buf[offset]); // update size is at third byte and is type size_t
            offset += sizeof(size_t);
            const uint8_t* resp = (update_size>0) ? ok_message : ko_message;
            bus->slaveWrite( resp, 2 ); // send response
            if( update_size>0 ) {
              if( Update.begin( update_size ) ) {
                processed_bytes = 0;
                log_d("Begin update %d bytes", update_size );
              }
            }
          }
          break;
          case REG_CMD_READ_DATA:
          {
            if( update_size == 0 ) {
              log_e("REG_CMD_READ_DATA received while update_size=0, aborting");
              return;
            }
            size_t recv_bytes = *((size_t*)&rcv_buf[offset]); // message size is at third byte and is type size_t
            offset += sizeof(size_t);
            if( recv_bytes == 0 || processed_bytes + recv_bytes > update_size ) {
              log_e("Bad length, aborting!");
              break;
            }

            uint8_t* bufptr = (uint8_t*)&rcv_buf[offset]; // buffer is after message size
            if( ! Update.write( bufptr, recv_bytes ) ) {
              log_e("Flash write fail, aborting!");
              break;
            }

            processed_bytes += recv_bytes;
            float fprogress = (float(processed_bytes)/float(update_size))*100.0;
            if( I2CUpdater::on_progress ) I2CUpdater::on_progress( fprogress, 100 );

            if( processed_bytes == update_size ) {
              log_d("Wrote %d bytes", processed_bytes );
              if ( !Update.end() ) {
                log_e( "Update failed");
                log_print_buf(bufptr, recv_bytes); // print last buffer
              } else {
                log_d( "OTA done!" );
                if ( Update.isFinished() ) {
                  log_d( "Update successfully completed." );
                }
              }
              ESP.restart();
            }
          }
          break;
          default:
            log_d("unknown command 0x%02x", reg_cmd );
          break;
        }
      }


    }; // end namespace Target

  }; // end namespace I2CUpdater

}; // end namespace SDUpdaterNS

