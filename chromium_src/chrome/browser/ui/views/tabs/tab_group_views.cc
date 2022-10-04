/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/views/tabs/brave_tab_group_header.h"
#include "brave/browser/ui/views/tabs/brave_tab_group_highlight.h"
#include "brave/browser/ui/views/tabs/brave_tab_group_underline.h"
#include "brave/browser/ui/views/tabs/features.h"

#define TabGroupHeader BraveTabGroupHeader
#define TabGroupUnderline BraveTabGroupUnderline
#define TabGroupHighlight BraveTabGroupHighlight
#define BRAVE_GET_LEADING_TRAILING_GROUP_VIEWS                             \
  if (tabs::features::ShouldShowVerticalTabs()) {                          \
    for (views::View * child : children) {                                 \
      TabSlotView* tab_slot_view = views::AsViewClass<TabSlotView>(child); \
      if (!tab_slot_view || tab_slot_view->group() != group_ ||            \
          !tab_slot_view->GetVisible()) {                                  \
        continue;                                                          \
      }                                                                    \
                                                                           \
      gfx::Rect child_widget_bounds =                                      \
          child->ConvertRectToWidget(child->GetLocalBounds());             \
                                                                           \
      if (!leading_child ||                                                \
          child_widget_bounds.y() < leading_child_widget_bounds.y()) {     \
        leading_child = child;                                             \
        leading_child_widget_bounds = child_widget_bounds;                 \
      }                                                                    \
                                                                           \
      if (!trailing_child || child_widget_bounds.bottom() >                \
                                 trailing_child_widget_bounds.bottom()) {  \
        trailing_child = child;                                            \
        trailing_child_widget_bounds = child_widget_bounds;                \
      }                                                                    \
    }                                                                      \
                                                                           \
    return {leading_child, trailing_child};                                \
  }

#include "src/chrome/browser/ui/views/tabs/tab_group_views.cc"

#undef BRAVE_GET_LEADING_TRAILING_GROUP_VIEWS
#undef TabGroupHighlight
#undef TabGroupUnderline
#undef TabGroupHeader
