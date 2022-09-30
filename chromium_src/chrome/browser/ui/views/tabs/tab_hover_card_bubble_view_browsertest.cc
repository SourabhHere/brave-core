/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chrome/browser/ui/tabs/tab_renderer_data.h"
#include "chrome/browser/ui/test/test_browser_dialog.h"
#include "chrome/browser/ui/views/tabs/tab.h"
#include "chrome/browser/ui/views/tabs/tab_hover_card_test_util.h"
#include "chrome/browser/ui/views/tabs/tab_strip.h"
#include "content/public/test/browser_test.h"
#include "url/gurl.h"

namespace {

constexpr char16_t kTabDomain[] = u"brave://settings";
constexpr char16_t kTabTitle[] = u"Settings - Addresses and more";
constexpr char kTabUrl[] = "chrome://settings/addresses";

}  // namespace

class TabHoverCardBubbleViewBrowserTest : public DialogBrowserTest,
                                          public test::TabHoverCardTestUtil {
 public:
  TabHoverCardBubbleViewBrowserTest() = default;

  TabHoverCardBubbleViewBrowserTest(const TabHoverCardBubbleViewBrowserTest&) =
      delete;
  TabHoverCardBubbleViewBrowserTest& operator=(
      const TabHoverCardBubbleViewBrowserTest&) = delete;

  ~TabHoverCardBubbleViewBrowserTest() override = default;

  // DialogBrowserTest:
  void ShowUi(const std::string& name) override {
    TabRendererData new_tab_data = TabRendererData();
    new_tab_data.title = kTabTitle;
    new_tab_data.last_committed_url = GURL(kTabUrl);
    GetTabStrip(browser())->AddTabAt(1, new_tab_data);

    SimulateHoverTab(browser(), 1);
  }
};

IN_PROC_BROWSER_TEST_F(TabHoverCardBubbleViewBrowserTest, ChromeSchemeUrl) {
  ShowUi("default");

  TabStrip* const tab_strip = GetTabStrip(browser());
  Tab* const tab = tab_strip->tab_at(1);
  ASSERT_NE(tab, nullptr);

  TabHoverCardBubbleView* const hover_card = GetHoverCard(tab_strip);
  ASSERT_NE(hover_card, nullptr);

  EXPECT_EQ(kTabTitle, hover_card->GetTitleTextForTesting());
  EXPECT_EQ(kTabDomain, hover_card->GetDomainTextForTesting());
}
