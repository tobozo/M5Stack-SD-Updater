#pragma once

#include "../misc/core.h"
#include "../misc/config.h"
#include "../Assets/Assets.hpp"

typedef void(*onselect_cb_t)(void);
typedef void(*onrender_cb_t)(void);
typedef void(*onidle_cb_t)(void);
typedef void(*onmodal_cb_t)(void);

struct MenuItemCallBacks
{
  onselect_cb_t onSelect; // on BtnA Click
  onrender_cb_t onRender; // on menuitem render
  onrender_cb_t onIdle;   // on idle (animations)
  onmodal_cb_t  onModal;  // on modal window (animations)
};


struct ButtonAction
{
  const char* title;
  onselect_cb_t onClick;
  RemoteAsset *asset;
};


struct MenuActionLabels
{
  const char *title;
  ButtonAction **Buttons;
};


class MenuGroup;

// UI Menu Item
class MenuAction
{
  public:
    MenuAction( const char* _title, MenuItemCallBacks* _callbacks, const RemoteAsset* _icon );
    ~MenuAction();
    void setTitle( const char* _title );
    void clear();
    char* title;
    MenuItemCallBacks* callbacks;
    const RemoteAsset* icon = nullptr;
    uint32_t textcolor = TEXT_COLOR;
  private:
    bool needs_free = false;
};

// UI Menu ItemCollection
class MenuGroup
{
  public:
    MenuGroup( MenuActionLabels* actionLabels, const char* _assets_folder = "" ); // empty menu group to be filled with push()
    ~MenuGroup();
    void push( const char* _title, MenuItemCallBacks* _callbacks, const RemoteAsset* _icon = nullptr, uint32_t _textcolor = TEXT_COLOR );
    void push( MenuAction *action );
    void setTitle( const char* title ) { Title = title; }
    void clear();
    size_t actions_count = 0;
    const char* Title = nullptr;
    MenuAction** Actions = nullptr;
    MenuActionLabels* ActionLabels = nullptr;
    const char* assets_folder = "";
    uint16_t selectedindex;
  private:
    bool needs_free = false;
};

