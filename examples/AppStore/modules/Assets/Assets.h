/*
 * Image Source: https://i.imgur.com/llemDHF.gif
 *
 * - ImageMagick:
 *    #> convert -strip disk.gif disk%02d.jpg
 *
 * - Crop visual to 30x30 in an image editor
 *
 * - Export to C uchar format:
 *    #> xxd -i disk00.jpg >> assets.h
 *    #> xxd -i disk01.jpg >> assets.h
 *
 *
 */

#pragma once


static const unsigned char disk01_jpg[1486] = {
  0xff, 0xd8, 0xff, 0xe0, 0x00, 0x10, 0x4a, 0x46, 0x49, 0x46, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x60, 0x00, 0x60, 0x00, 0x00, 0xff, 0xdb, 0x00, 0x43,
  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0xff, 0xdb, 0x00, 0x43, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0xff, 0xc0, 0x00, 0x11, 0x08, 0x00, 0x1e, 0x00, 0x1e, 0x03,
  0x01, 0x22, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01, 0xff, 0xc4, 0x00,
  0x1f, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0xff, 0xc4, 0x00, 0xb5, 0x10, 0x00,
  0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00,
  0x00, 0x01, 0x7d, 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21,
  0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32, 0x81,
  0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0, 0x24,
  0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x25,
  0x26, 0x27, 0x28, 0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,
  0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x53, 0x54, 0x55, 0x56,
  0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,
  0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83, 0x84, 0x85, 0x86,
  0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
  0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3,
  0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6,
  0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
  0xda, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1,
  0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xff, 0xc4, 0x00,
  0x1f, 0x01, 0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0xff, 0xc4, 0x00, 0xb5, 0x11, 0x00,
  0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00,
  0x01, 0x02, 0x77, 0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31,
  0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81, 0x08,
  0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0, 0x15,
  0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25, 0xf1, 0x17, 0x18,
  0x19, 0x1a, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38, 0x39,
  0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x53, 0x54, 0x55,
  0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
  0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x82, 0x83, 0x84,
  0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
  0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,
  0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4,
  0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
  0xd8, 0xd9, 0xda, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
  0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xff, 0xda, 0x00,
  0x0c, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03, 0x11, 0x00, 0x3f, 0x00, 0xfe,
  0xc7, 0x7f, 0x6f, 0xc0, 0x7f, 0xe1, 0x45, 0x78, 0x04, 0x16, 0x04, 0xaf,
  0xed, 0xaf, 0xff, 0x00, 0x04, 0xd4, 0x0d, 0xb7, 0x20, 0x93, 0xff, 0x00,
  0x0f, 0x16, 0xfd, 0x95, 0xf2, 0x06, 0x64, 0x24, 0x65, 0x8a, 0xb9, 0x0e,
  0x64, 0x25, 0x41, 0x03, 0x2c, 0x55, 0xc7, 0xa9, 0xfe, 0xd7, 0x7a, 0xff,
  0x00, 0xc5, 0x4f, 0x09, 0xfe, 0xc9, 0xff, 0x00, 0xb4, 0xff, 0x00, 0x8a,
  0xbe, 0x05, 0x43, 0xad, 0xdc, 0xfc, 0x6f, 0xf0, 0xcf, 0xec, 0xf1, 0xf1,
  0xab, 0xc4, 0x1f, 0x07, 0x2d, 0xfc, 0x37, 0xe1, 0xa8, 0xfc, 0x6d, 0xe2,
  0x29, 0xfe, 0x2a, 0xe8, 0xdf, 0x0d, 0xbc, 0x4b, 0xa8, 0xfc, 0x3d, 0x87,
  0x40, 0xf0, 0x64, 0xfa, 0x66, 0xb7, 0x07, 0x8b, 0xb5, 0xa9, 0x7c, 0x5d,
  0x6d, 0xa4, 0x26, 0x97, 0xe1, 0x89, 0xb4, 0x5d, 0x5e, 0x2d, 0x7e, 0xf9,
  0xa0, 0xd2, 0xa4, 0xd2, 0xef, 0xd2, 0xec, 0xda, 0x4b, 0xe4, 0xff, 0x00,
  0xb4, 0xaf, 0xc5, 0x3f, 0x0c, 0xeb, 0xba, 0xb6, 0xbb, 0xfb, 0x33, 0xc7,
  0xfb, 0x34, 0xfc, 0x6c, 0xfd, 0xaa, 0x35, 0xcf, 0xf8, 0x42, 0xbc, 0x0f,
  0xf1, 0x2f, 0xe2, 0x27, 0x86, 0x3e, 0x0c, 0x6b, 0xff, 0x00, 0x07, 0xbc,
  0x0d, 0xaa, 0x7c, 0x2a, 0xd0, 0xbc, 0x4b, 0xe3, 0x2f, 0x14, 0xc5, 0xf0,
  0x4b, 0xc7, 0xb0, 0xf8, 0xf3, 0xe3, 0x37, 0xc7, 0xdf, 0xd9, 0xda, 0xeb,
  0x43, 0xf1, 0x9d, 0xd7, 0xc4, 0x0f, 0x83, 0x1e, 0x39, 0xf1, 0x17, 0xc2,
  0xef, 0x15, 0xfc, 0x12, 0xf1, 0x4e, 0xa7, 0xf1, 0x1b, 0xe1, 0x6f, 0x8d,
  0x7e, 0x13, 0x58, 0xf8, 0xde, 0xe2, 0xf3, 0xe1, 0xb6, 0xb3, 0x27, 0xc3,
  0x6d, 0x6f, 0x5d, 0xf8, 0xc0, 0xfc, 0x2b, 0x38, 0x38, 0xfd, 0x84, 0x3f,
  0xe0, 0xb5, 0xd9, 0xc1, 0xc6, 0x7f, 0xe0, 0xac, 0x8a, 0xc3, 0x3d, 0xbe,
  0x57, 0xff, 0x00, 0x82, 0xd2, 0x3a, 0x37, 0xd1, 0xd1, 0x94, 0xf4, 0x65,
  0x61, 0x90, 0x40, 0x3c, 0xa3, 0xc4, 0x9e, 0x2e, 0xf0, 0x06, 0xb7, 0x69,
  0xf1, 0x2b, 0xc3, 0xff, 0x00, 0xb3, 0x97, 0xc6, 0xbd, 0x6f, 0xe3, 0xef,
  0xec, 0xa3, 0xa0, 0xfe, 0xd0, 0xff, 0x00, 0xf0, 0x44, 0xad, 0x67, 0xc2,
  0x9e, 0x39, 0x9b, 0xf6, 0x89, 0xf1, 0xb7, 0xed, 0x67, 0xe1, 0xcb, 0x2f,
  0xda, 0x17, 0xc4, 0xbf, 0xf0, 0x54, 0x2d, 0x2a, 0xcb, 0xe3, 0x8f, 0x83,
  0xb4, 0xaf, 0x8f, 0xde, 0x3d, 0xf1, 0xef, 0xc5, 0x8f, 0x12, 0x1d, 0x63,
  0x47, 0xf8, 0x6d, 0xe1, 0x7f, 0xd9, 0x9b, 0x5a, 0xd7, 0xbe, 0x10, 0x43,
  0xf1, 0x02, 0x5d, 0x23, 0xc0, 0x36, 0x3e, 0x22, 0xf0, 0xcf, 0x8c, 0x34,
  0xff, 0x00, 0x07, 0xf8, 0x7a, 0xe3, 0xe2, 0xd6, 0xab, 0xad, 0x78, 0xd7,
  0xeb, 0xcf, 0xf8, 0x28, 0xa7, 0xfc, 0x12, 0x1f, 0xf6, 0x42, 0xff, 0x00,
  0x82, 0x9b, 0xb7, 0x81, 0x35, 0x5f, 0x8f, 0x5a, 0x6f, 0x8c, 0xfc, 0x27,
  0xf1, 0x07, 0xe1, 0xd8, 0x97, 0x4e, 0xd0, 0xbe, 0x2c, 0xfc, 0x21, 0xd4,
  0x7c, 0x2b, 0xe1, 0xbf, 0x88, 0x77, 0x9e, 0x0d, 0x95, 0xb5, 0x3b, 0x89,
  0xbe, 0x1c, 0xf8, 0x8f, 0x50, 0xf1, 0x67, 0x82, 0xfc, 0x79, 0xa1, 0xf8,
  0x8b, 0xc1, 0xb6, 0xfa, 0xe6, 0xa5, 0x27, 0x89, 0x34, 0x8b, 0x4d, 0x5b,
  0xc3, 0xf7, 0x3a, 0xaf, 0x85, 0xf5, 0xc9, 0x35, 0x5b, 0x9f, 0x05, 0xeb,
  0x5e, 0x1e, 0xb2, 0xf1, 0x8f, 0x8f, 0xec, 0x3c, 0x55, 0xe6, 0x3e, 0x1c,
  0xf0, 0x1c, 0x5e, 0x18, 0xf1, 0x26, 0x81, 0xe2, 0x67, 0xff, 0x00, 0x82,
  0x70, 0xff, 0x00, 0xc1, 0x57, 0x3c, 0x75, 0x71, 0xe1, 0xad, 0x67, 0x4b,
  0xf1, 0x26, 0x8f, 0xa0, 0xfc, 0x62, 0xff, 0x00, 0x82, 0x82, 0x7c, 0x2b,
  0xf8, 0xeb, 0xf0, 0xfe, 0x3f, 0x11, 0xe8, 0x17, 0xf1, 0x6b, 0x7e, 0x18,
  0xd7, 0xf5, 0x0f, 0x86, 0x1f, 0x1a, 0xff, 0x00, 0xe0, 0xae, 0x7e, 0x3d,
  0xf8, 0x6f, 0xab, 0x6b, 0xbe, 0x10, 0xf1, 0x1d, 0x8e, 0x97, 0xe2, 0xdf,
  0x05, 0xeb, 0x7a, 0xbf, 0x85, 0x35, 0x1d, 0x4f, 0xc1, 0xbe, 0x34, 0xd1,
  0xbc, 0x3d, 0xe3, 0x4f, 0x0c, 0x5c, 0x69, 0x3e, 0x2d, 0xf0, 0xee, 0x85,
  0xad, 0x69, 0xff, 0x00, 0xa4, 0x9f, 0x03, 0x7e, 0x36, 0xe9, 0x9f, 0x1a,
  0xf4, 0xcf, 0x1a, 0xff, 0x00, 0xc5, 0x19, 0xe3, 0x8f, 0x86, 0x9e, 0x33,
  0xf8, 0x63, 0xe3, 0x86, 0xf8, 0x75, 0xf1, 0x47, 0xe1, 0x77, 0xc4, 0x41,
  0xe0, 0xf9, 0xbc, 0x5f, 0xf0, 0xf7, 0xc6, 0x73, 0x78, 0x3f, 0xc1, 0xff,
  0x00, 0x12, 0x34, 0x8d, 0x1f, 0x54, 0xd5, 0xbe, 0x1b, 0x78, 0xd3, 0xe2,
  0x37, 0xc3, 0x7d, 0x69, 0x35, 0xef, 0x86, 0x3f, 0x11, 0x3e, 0x1e, 0x7c,
  0x40, 0xd3, 0xaf, 0x3c, 0x0d, 0xf1, 0x03, 0xc5, 0xfa, 0x55, 0x9e, 0x93,
  0xe3, 0x4d, 0x3f, 0x43, 0xd6, 0x75, 0x0d, 0x23, 0xc6, 0xda, 0x3f, 0x8b,
  0x3c, 0x25, 0xe1, 0xb0, 0x0f, 0x1b, 0xf0, 0x18, 0x3f, 0xf0, 0xf1, 0xaf,
  0xda, 0xa8, 0xe3, 0x8f, 0xf8, 0x62, 0x8f, 0xd8, 0x09, 0x73, 0xdb, 0x70,
  0xf8, 0xe9, 0xff, 0x00, 0x05, 0x2a, 0x24, 0x67, 0xd4, 0x06, 0x52, 0x47,
  0x50, 0x19, 0x4f, 0x42, 0x2b, 0xed, 0x8a, 0xf8, 0x8f, 0xc0, 0x60, 0xff,
  0x00, 0xc3, 0xc6, 0x3f, 0x6a, 0x77, 0xda, 0xa4, 0xb7, 0xec, 0x55, 0xfb,
  0x02, 0x23, 0x64, 0xf3, 0x94, 0xf8, 0xe7, 0xff, 0x00, 0x05, 0x27, 0x3b,
  0x4b, 0x6c, 0x25, 0xd1, 0x09, 0x92, 0x48, 0xd8, 0xed, 0x7d, 0xee, 0xc1,
  0xbe, 0x56, 0x1e, 0x5f, 0xdb, 0x7c, 0xfa, 0x0f, 0xcc, 0xff, 0x00, 0x85,
  0x00, 0x2d, 0x7c, 0x4f, 0xfb, 0x2b, 0x02, 0x3e, 0x3a, 0xff, 0x00, 0xc1,
  0x4a, 0xf2, 0x31, 0x9f, 0xdb, 0x5f, 0xc0, 0x6c, 0x33, 0xdd, 0x7f, 0xe1,
  0xdc, 0xdf, 0xb0, 0x12, 0xe4, 0x7a, 0x8d, 0xca, 0xc3, 0x3d, 0x32, 0xa4,
  0x75, 0x06, 0xbe, 0xd6, 0x3b, 0xb0, 0x70, 0xaa, 0x4e, 0x0f, 0x05, 0x88,
  0x07, 0xd8, 0x9d, 0xa7, 0x00, 0xf7, 0xe0, 0xfd, 0x0d, 0x7c, 0x49, 0xfb,
  0x2b, 0xab, 0x7f, 0xc2, 0xf3, 0xff, 0x00, 0x82, 0x94, 0x15, 0x0b, 0xba,
  0x4f, 0xdb, 0x57, 0xc0, 0x92, 0x7d, 0xf2, 0x99, 0x03, 0xfe, 0x09, 0xd3,
  0xfb, 0x02, 0xa8, 0x77, 0x75, 0x8c, 0x93, 0x21, 0x50, 0x8a, 0x54, 0x2e,
  0xc0, 0x88, 0x80, 0x12, 0x54, 0xb3, 0x80, 0x7f, 0xff, 0xd9
};
static const unsigned int disk01_jpg_len = 1486;


// 16 x 18
static const unsigned char broken_png[] = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
  0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x12,
  0x08, 0x06, 0x00, 0x00, 0x00, 0x52, 0x3b, 0x5e, 0x6a, 0x00, 0x00, 0x00,
  0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x0e, 0xc4, 0x00, 0x00, 0x0e,
  0xc4, 0x01, 0x95, 0x2b, 0x0e, 0x1b, 0x00, 0x00, 0x02, 0x9f, 0x49, 0x44,
  0x41, 0x54, 0x38, 0x8d, 0x85, 0x91, 0x5f, 0x48, 0x53, 0x61, 0x1c, 0x86,
  0x9f, 0x63, 0xe7, 0xb0, 0x69, 0x63, 0xb8, 0x94, 0x0d, 0x74, 0x61, 0x24,
  0xa9, 0x24, 0x76, 0x91, 0x19, 0x06, 0x21, 0x08, 0x11, 0x9c, 0x42, 0x21,
  0x42, 0x22, 0x08, 0xba, 0x49, 0x54, 0x90, 0xae, 0x22, 0x2c, 0xa2, 0xae,
  0xba, 0xe8, 0x0f, 0x24, 0x08, 0x82, 0x90, 0x09, 0x26, 0x06, 0xa6, 0x84,
  0x41, 0x64, 0x9a, 0x90, 0x56, 0x0a, 0xa9, 0x9b, 0x0c, 0x5d, 0x2c, 0x19,
  0x1e, 0x54, 0x2a, 0x25, 0xb5, 0xb6, 0xf4, 0x6c, 0x3b, 0xf3, 0x9c, 0x2e,
  0x64, 0xb1, 0x75, 0x24, 0xbf, 0xcb, 0xf7, 0x7b, 0x7f, 0xcf, 0xef, 0xcf,
  0x2b, 0x60, 0xc5, 0x10, 0xad, 0x22, 0x02, 0x02, 0xbb, 0xbd, 0xc6, 0x2b,
  0x8d, 0x14, 0x1d, 0x2a, 0xaa, 0xad, 0xab, 0xab, 0x7b, 0x9c, 0xd0, 0x04,
  0x51, 0x14, 0x8d, 0xd1, 0xd1, 0x51, 0x5c, 0x2e, 0xd7, 0xae, 0x00, 0x8f,
  0xc7, 0xc3, 0xda, 0xda, 0xda, 0x9a, 0xae, 0xeb, 0xf7, 0x1a, 0x1a, 0x1a,
  0xee, 0x03, 0x08, 0x92, 0x24, 0x19, 0x8a, 0xa2, 0x10, 0x8f, 0xc7, 0xd9,
  0xdc, 0xdc, 0xfc, 0x2f, 0xc0, 0xe7, 0xf3, 0x61, 0xb7, 0xdb, 0x71, 0x38,
  0x1c, 0x4b, 0xd3, 0xd3, 0xd3, 0x9f, 0xea, 0xeb, 0xeb, 0xcf, 0x8b, 0x89,
  0x4f, 0x55, 0x55, 0xd9, 0xd8, 0xd8, 0x00, 0x60, 0x36, 0x3c, 0x4b, 0xe7,
  0x4a, 0x27, 0x36, 0x6c, 0xdc, 0xc8, 0xbb, 0x81, 0x28, 0x6e, 0xdb, 0xb6,
  0xb6, 0xb6, 0xb0, 0x5a, 0xad, 0x14, 0x14, 0x14, 0xb8, 0x6d, 0x76, 0x9b,
  0xa3, 0xad, 0xad, 0xad, 0x47, 0xfc, 0xb7, 0xcb, 0x54, 0x68, 0x8a, 0x5e,
  0xbd, 0x97, 0xe1, 0xdc, 0x61, 0xac, 0x5e, 0x2b, 0xd9, 0xc3, 0xd9, 0x54,
  0x9f, 0xad, 0x26, 0x37, 0x37, 0x17, 0xbf, 0xdf, 0xcf, 0xe0, 0xe0, 0x20,
  0x6e, 0xb7, 0x9b, 0x48, 0x24, 0xb2, 0x77, 0x6e, 0x7e, 0xae, 0x26, 0xcd,
  0xb4, 0xe7, 0x6f, 0x0f, 0x43, 0xbf, 0x86, 0x00, 0x88, 0xc4, 0x22, 0xb4,
  0x77, 0xb4, 0xb3, 0xb2, 0xb2, 0x02, 0x80, 0xc5, 0x62, 0x61, 0x7d, 0x7d,
  0x9d, 0x40, 0x20, 0xc0, 0xfc, 0xfc, 0x3c, 0xfd, 0x03, 0xfd, 0x98, 0x26,
  0x70, 0xa7, 0xbb, 0x29, 0x5c, 0x2a, 0x24, 0xf0, 0x21, 0x00, 0x2a, 0x64,
  0xef, 0xcb, 0x46, 0x92, 0x24, 0x00, 0x64, 0x59, 0x46, 0x96, 0x65, 0x6c,
  0x36, 0x1b, 0x9a, 0xa6, 0xd1, 0x37, 0xd0, 0x67, 0x06, 0xc8, 0x0e, 0x19,
  0xcb, 0x6f, 0x0b, 0xd7, 0x1f, 0x5d, 0x47, 0x92, 0x24, 0xba, 0x5f, 0x75,
  0x13, 0x0e, 0xef, 0x27, 0x18, 0x4c, 0x38, 0x96, 0xc9, 0xc9, 0x51, 0x97,
  0xb2, 0x32, 0xb3, 0x16, 0xd9, 0x4a, 0x4a, 0x21, 0x1c, 0x0e, 0xa3, 0x2e,
  0x2f, 0x63, 0xa4, 0xa5, 0xa1, 0xa7, 0xa7, 0xa7, 0x40, 0xab, 0xaa, 0x8e,
  0xf2, 0x4d, 0x15, 0x20, 0x0a, 0xa8, 0x2d, 0xc0, 0xd5, 0xbb, 0xc0, 0x2d,
  0x80, 0x94, 0x1b, 0xb8, 0x9b, 0x9b, 0x71, 0x75, 0x75, 0xed, 0x18, 0x21,
  0x2f, 0x80, 0x4b, 0x66, 0x39, 0xf5, 0x88, 0xf1, 0x16, 0x72, 0x9e, 0xe4,
  0x70, 0xe0, 0xe6, 0x4d, 0xb3, 0xf1, 0x1c, 0xf0, 0xf4, 0x3d, 0xe0, 0xa8,
  0x4d, 0x74, 0x37, 0x03, 0xb0, 0x80, 0x56, 0x47, 0xe6, 0xc7, 0x0b, 0x1c,
  0xbc, 0x76, 0x2d, 0x49, 0x3f, 0x8d, 0xfe, 0xb3, 0x14, 0x22, 0x0f, 0x2f,
  0xc2, 0xf7, 0x67, 0x86, 0x61, 0x14, 0xfb, 0x7c, 0xbe, 0x1e, 0x32, 0xe8,
  0x31, 0xc5, 0x08, 0x2e, 0xf6, 0xa8, 0x67, 0xb0, 0x4f, 0x5c, 0x26, 0xbf,
  0xa9, 0x09, 0x34, 0x0d, 0x49, 0xfa, 0x72, 0x12, 0x3c, 0x27, 0xe0, 0xe5,
  0x6b, 0xaf, 0xf7, 0xd4, 0x61, 0x45, 0x51, 0xee, 0x08, 0x82, 0x50, 0x83,
  0x40, 0x4d, 0x4a, 0x0a, 0x1d, 0x42, 0x07, 0x7e, 0xfc, 0xa0, 0x43, 0xe6,
  0x46, 0xda, 0xe7, 0x87, 0x13, 0x13, 0xb7, 0x0d, 0x41, 0x60, 0x61, 0x61,
  0x61, 0x0c, 0x30, 0x00, 0x74, 0x5d, 0x2f, 0x8e, 0x46, 0xa3, 0x35, 0x89,
  0x9a, 0x14, 0x40, 0xec, 0x78, 0x8c, 0x99, 0xaf, 0x33, 0x23, 0x93, 0x93,
  0x93, 0x53, 0xc0, 0x5c, 0xf3, 0x2f, 0x7a, 0x29, 0x2f, 0x07, 0xc0, 0xeb,
  0xf5, 0x96, 0xc5, 0xe3, 0xf1, 0x3c, 0xc3, 0x30, 0xca, 0x92, 0x6b, 0xfe,
  0xc6, 0x18, 0x8b, 0xc5, 0x9e, 0xaf, 0xae, 0xae, 0x2e, 0x0a, 0x82, 0x30,
  0x58, 0x5a, 0x5a, 0xfa, 0x26, 0xd9, 0x64, 0x18, 0x46, 0x71, 0x30, 0x18,
  0x7c, 0xa0, 0x69, 0x9a, 0x0c, 0x90, 0x91, 0x91, 0x41, 0x28, 0x14, 0xa2,
  0xa4, 0xbc, 0x04, 0x51, 0xd7, 0x75, 0xc6, 0xc7, 0xc7, 0xc9, 0xca, 0xcf,
  0x6a, 0xad, 0x3c, 0x56, 0xf9, 0xce, 0x7c, 0x13, 0x18, 0x1b, 0x1d, 0x6b,
  0xd5, 0x0c, 0xad, 0x22, 0x59, 0x53, 0x14, 0x65, 0x7b, 0x02, 0xa7, 0xd3,
  0xb9, 0xbd, 0xdb, 0x11, 0xbd, 0xf2, 0xc7, 0xdb, 0x1f, 0x3b, 0x02, 0x9c,
  0x4e, 0xe7, 0x08, 0x50, 0x61, 0xfa, 0xb0, 0xc1, 0x1f, 0xc0, 0xbf, 0x14,
  0x66, 0xb5, 0xad, 0x80, 0xfb, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e,
  0x44, 0xae, 0x42, 0x60, 0x82
};
static const unsigned int broken_png_len = 749;