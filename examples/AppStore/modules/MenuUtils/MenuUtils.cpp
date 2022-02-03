#pragma once

#include "MenuUtils.hpp"


MenuAction::MenuAction( const char* _title, MenuItemCallBacks* _callbacks, const RemoteAsset* _icon )
{
  //setTitle( _title );
  title     = (char*)_title;
  icon      = _icon;
  callbacks = _callbacks;
  needs_free = false;
}


MenuAction::~MenuAction()
{
  clear();
}


void MenuAction::setTitle( const char* _title )
{
  if( !_title || _title[0]=='\0' ) return;
  size_t title_len = strlen( _title );
  title = (char*)calloc( title_len+1, sizeof(char));
  if( title == NULL ) {
    log_e("Failed to alloc %d bytes for string %s", title_len+1, _title );
    return;
  }
  memcpy( title, _title, title_len );
  log_v("Allocated %d bytes for title %s", title_len+1, _title );
  needs_free = true;
}

void MenuAction::clear()
{
  if( needs_free ) {
    log_v("Clearing %s", title );
    free( title );
    title = nullptr;
  }
  needs_free = false;
}




MenuGroup::MenuGroup( MenuActionLabels* actionLabels, const char* _assets_folder )
{
  ActionLabels = actionLabels;
  Title = ActionLabels->title;
  assets_folder = _assets_folder;
}


MenuGroup::~MenuGroup()
{
  clear();
}

void MenuGroup::clear()
{
  if( actions_count == 0 || !Actions || needs_free == false ) {
    log_v("Actions for menu %s do not need clearing", Title );
    return;
  }
  size_t before_free = ESP.getFreeHeap();
  log_v("Clearing actions for menu %s", Title );
  for( int i=0; i<actions_count; i++ ) {
    log_v("Clearing action #%d (%s)", i, Actions[i]->title );
    Actions[i]->clear();
    free( Actions[i] );
  }
  if( Actions ) {
    free( Actions );
  }
  log_v("%d bytes freed", ESP.getFreeHeap() - before_free );
  actions_count = 0;
  Actions = nullptr;
  needs_free = false;
}


void MenuGroup::push( MenuAction *action )
{
  push( action->title, action->callbacks, action->icon, action->textcolor );
}


void MenuGroup::push( const char* _title, MenuItemCallBacks* _callbacks, const RemoteAsset* _icon, uint32_t _textcolor )
{
  if( actions_count == 0 || Actions == nullptr ) {
    Actions = (MenuAction**)calloc( 1, sizeof( MenuAction* ) );
  } else {
    Actions = (MenuAction**)realloc( Actions, (actions_count+1) * (sizeof( MenuAction* )) );
  }
  log_v("Pushing action #%d %s", actions_count, _title?_title:"" );
  if( Actions == NULL ) {
    log_e("Failed to allocate %d bytes for menu Actions", (actions_count+1) * sizeof( MenuAction* ) );
    clear();
    return;
  }
  needs_free = true;
  Actions[actions_count] = (MenuAction*)calloc( 1, sizeof( MenuAction ) );
  Actions[actions_count]->setTitle( _title );
  Actions[actions_count]->icon      = _icon;
  Actions[actions_count]->callbacks = _callbacks;
  Actions[actions_count]->textcolor = _textcolor;
  actions_count++;
}




