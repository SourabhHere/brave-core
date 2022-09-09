/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_VIEWS_BRAVE_ACTIONS_BRAVE_ACTIONS_CONTAINER_H_
#define BRAVE_BROWSER_UI_VIEWS_BRAVE_ACTIONS_BRAVE_ACTIONS_CONTAINER_H_

#include <memory>

#include "base/gtest_prod_util.h"
#include "base/memory/raw_ptr.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/views/toolbar/toolbar_action_view.h"
#include "components/prefs/pref_member.h"
#include "ui/gfx/skia_util.h"
#include "ui/views/view.h"

class BraveActionViewController;
class BraveActionsContainerTest;
class BraveRewardsActionView;
class BraveShieldsActionView;
class RewardsBrowserTest;
class BraveNewsActionView;

namespace policy {
FORWARD_DECLARE_TEST(BraveRewardsPolicyTest, RewardsIconIsHidden);
}

namespace views {
class Button;
}

// This View contains all the built-in BraveActions such as Shields and Payments
// TODO(petemill): consider splitting to separate model, like
// ToolbarActionsModel and ToolbarActionsBar
class BraveActionsContainer : public views::View,
                              public ToolbarActionView::Delegate {
 public:
  BraveActionsContainer(Browser* browser, Profile* profile);
  BraveActionsContainer(const BraveActionsContainer&) = delete;
  BraveActionsContainer& operator=(const BraveActionsContainer&) = delete;
  ~BraveActionsContainer() override;
  void Init();
  void Update();
  void SetShouldHide(bool should_hide);
  // ToolbarActionView::Delegate
  content::WebContents* GetCurrentWebContents() override;
  // Returns the view of the toolbar actions overflow menu to use as a
  // reference point for a popup when this view isn't visible.
  views::LabelButton* GetOverflowReferenceView() const override;
  // Returns the preferred size of the ToolbarActionView.
  gfx::Size GetToolbarActionSize() override;
  // Overridden from views::DragController (required by
  // ToolbarActionView::Delegate):
  void WriteDragDataForView(View* sender,
                            const gfx::Point& press_pt,
                            ui::OSExchangeData* data) override;
  int GetDragOperationsForView(View* sender, const gfx::Point& p) override;
  bool CanStartDragForView(View* sender,
                           const gfx::Point& press_pt,
                           const gfx::Point& p) override;

  // Brave Rewards preferences change observers callback.
  void OnBraveRewardsPreferencesChanged();

  // views::View:
  void ChildPreferredSizeChanged(views::View* child) override;

  BraveShieldsActionView* GetShieldsActionView() { return shields_action_btn_; }

 private:
  FRIEND_TEST_ALL_PREFIXES(policy::BraveRewardsPolicyTest, RewardsIconIsHidden);
  friend class ::BraveActionsContainerTest;
  friend class ::RewardsBrowserTest;

  bool ShouldShowBraveRewardsAction() const;
  void AddActionViewForRewards();
  void AddActionViewForShields();
  void AddActionViewForNews();

  void UpdateVisibility();

  bool should_hide_ = false;

  // The Browser this LocationBarView is in.  Note that at least
  // chromeos::SimpleWebViewDialog uses a LocationBarView outside any browser
  // window, so this may be NULL.
  raw_ptr<Browser> browser_ = nullptr;

  raw_ptr<BraveShieldsActionView> shields_action_btn_ = nullptr;
  raw_ptr<BraveRewardsActionView> rewards_action_btn_ = nullptr;
  raw_ptr<BraveNewsActionView> news_action_btn_ = nullptr;

  // Listen for Brave Rewards preferences changes.
  BooleanPrefMember show_brave_rewards_button_;

  base::WeakPtrFactory<BraveActionsContainer> weak_ptr_factory_{this};
};

#endif  // BRAVE_BROWSER_UI_VIEWS_BRAVE_ACTIONS_BRAVE_ACTIONS_CONTAINER_H_
