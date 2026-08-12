// Copyright (c) 2026 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "base/run_loop.h"
#include "base/test/run_until.h"
#include "brave/browser/user_control/user_control_policy.h"
#include "chrome/browser/content_settings/host_content_settings_map_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/content_settings/core/common/content_settings.h"
#include "components/content_settings/core/common/content_settings_types.h"
#include "components/javascript_dialogs/app_modal_dialog_controller.h"
#include "components/javascript_dialogs/app_modal_dialog_queue.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_navigation_observer.h"
#include "content/public/test/test_utils.h"
#include "net/base/filename_util.h"
#include "net/dns/mock_host_resolver.h"
#include "net/test/embedded_test_server/embedded_test_server.h"
#include "url/gurl.h"

namespace user_control {
namespace {

class PageExitBrowserTest : public InProcessBrowserTest {
 public:
  void SetUpOnMainThread() override {
    InProcessBrowserTest::SetUpOnMainThread();
    host_resolver()->AddRule("*", "127.0.0.1");
    embedded_test_server()->ServeFilesFromSourceDirectory(
        GetChromeTestDataDir());
    ASSERT_TRUE(embedded_test_server()->Start());
  }

 protected:
  HostContentSettingsMap* settings(Profile* profile = nullptr) {
    return HostContentSettingsMapFactory::GetForProfile(
        profile ? profile : browser()->profile());
  }

  void SetException(const GURL& url,
                    ContentSettingsType type,
                    ContentSetting setting) {
    settings()->SetContentSettingDefaultScope(url, url, type, setting);
  }

  content::WebContents* OpenPopup(const GURL& url) {
    content::WebContentsAddedObserver added_observer;
    content::TestNavigationObserver navigation_observer(nullptr, 1);
    navigation_observer.StartWatchingNewWebContents();
    EXPECT_TRUE(content::ExecJs(
        browser()->tab_strip_model()->GetActiveWebContents(),
        content::JsReplace("window.open($1, 'takeback_popup');", url)));
    content::WebContents* popup = added_observer.GetWebContents();
    navigation_observer.WaitForNavigationFinished();
    EXPECT_EQ(url, popup->GetLastCommittedURL());
    return popup;
  }

  content::WebContents* OpenBlankPopup(content::WebContents* opener) {
    content::WebContentsAddedObserver added_observer;
    EXPECT_TRUE(content::ExecJs(
        opener,
        "window.takebackPopup = window.open('', '', 'width=200,height=100');"));
    return added_observer.GetWebContents();
  }

  void InstallBeforeUnloadHandler(content::WebContents* web_contents) {
    ASSERT_TRUE(
        content::ExecJs(web_contents,
                        "window.addEventListener('beforeunload', event => {"
                        "  event.preventDefault();"
                        "  event.returnValue = '';"
                        "});"));
    content::PrepContentsForBeforeUnloadTest(web_contents);
  }

  void CancelActiveDialog() {
    auto* queue = javascript_dialogs::AppModalDialogQueue::GetInstance();
    ASSERT_TRUE(queue->HasActiveDialog());
    queue->active_dialog()->OnCancel(true);
    ASSERT_TRUE(
        base::test::RunUntil([queue] { return !queue->HasActiveDialog(); }));
  }
};

IN_PROC_BROWSER_TEST_F(PageExitBrowserTest,
                       WindowCloseProtectedThenCategoryAllowAfterReload) {
  const GURL url = embedded_test_server()->GetURL("a.test", "/title1.html");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));
  content::WebContents* popup = OpenPopup(url);

  EXPECT_TRUE(content::ExecJs(popup, "window.close();"));
  base::RunLoop().RunUntilIdle();
  EXPECT_FALSE(popup->IsBeingDestroyed());
  EXPECT_EQ(false, content::EvalJs(popup, "window.closed"));

  SetException(url, ContentSettingsType::BRAVE_USER_CONTROL_PAGE_EXIT,
               CONTENT_SETTING_ALLOW);
  ASSERT_TRUE(content::NavigateToURL(popup, url.Resolve("/title2.html")));
  content::WebContentsDestroyedWatcher destroyed_watcher(popup);
  EXPECT_TRUE(content::ExecJs(popup, "window.close();"));
  destroyed_watcher.Wait();
}

IN_PROC_BROWSER_TEST_F(PageExitBrowserTest, WindowCloseMasterAllow) {
  const GURL url = embedded_test_server()->GetURL("a.test", "/title1.html");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));
  SetException(url, ContentSettingsType::BRAVE_USER_CONTROL,
               CONTENT_SETTING_ALLOW);
  content::WebContents* popup = OpenPopup(url);

  content::WebContentsDestroyedWatcher destroyed_watcher(popup);
  EXPECT_TRUE(content::ExecJs(popup, "window.close();"));
  destroyed_watcher.Wait();
}

IN_PROC_BROWSER_TEST_F(PageExitBrowserTest,
                       WebPageCannotCloseBlankPopupByDefault) {
  const GURL url = embedded_test_server()->GetURL("a.test", "/title1.html");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));
  content::WebContents* opener =
      browser()->tab_strip_model()->GetActiveWebContents();
  content::WebContents* popup = OpenBlankPopup(opener);
  ASSERT_TRUE(popup);

  EXPECT_TRUE(content::ExecJs(opener, "window.takebackPopup.close();"));
  base::RunLoop().RunUntilIdle();
  EXPECT_FALSE(popup->IsBeingDestroyed());
  EXPECT_EQ(false, content::EvalJs(opener, "window.takebackPopup.closed"));
}

IN_PROC_BROWSER_TEST_F(PageExitBrowserTest,
                       LocalFileCannotCloseBlankPopupByDefault) {
  const GURL file_url =
      net::FilePathToFileURL(GetChromeTestDataDir().AppendASCII("title1.html"));
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), file_url));
  content::WebContents* opener =
      browser()->tab_strip_model()->GetActiveWebContents();
  content::WebContents* popup = OpenBlankPopup(opener);
  ASSERT_TRUE(popup);

  EXPECT_TRUE(content::ExecJs(opener, "window.takebackPopup.close();"));
  base::RunLoop().RunUntilIdle();
  EXPECT_FALSE(popup->IsBeingDestroyed());
  EXPECT_EQ(false, content::EvalJs(opener, "window.takebackPopup.closed"));
}

IN_PROC_BROWSER_TEST_F(PageExitBrowserTest, BeforeUnloadProtectedByDefault) {
  const GURL url = embedded_test_server()->GetURL("a.test", "/title1.html");
  const GURL target = embedded_test_server()->GetURL("a.test", "/title2.html");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));
  content::WebContents* web_contents =
      browser()->tab_strip_model()->GetActiveWebContents();
  InstallBeforeUnloadHandler(web_contents);

  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), target));
  EXPECT_EQ(target, web_contents->GetLastCommittedURL());
  EXPECT_FALSE(javascript_dialogs::AppModalDialogQueue::GetInstance()
                   ->HasActiveDialog());
}

IN_PROC_BROWSER_TEST_F(PageExitBrowserTest, BeforeUnloadCategoryAllow) {
  const GURL url = embedded_test_server()->GetURL("a.test", "/title1.html");
  const GURL target = embedded_test_server()->GetURL("a.test", "/title2.html");
  SetException(url, ContentSettingsType::BRAVE_USER_CONTROL_PAGE_EXIT,
               CONTENT_SETTING_ALLOW);
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));
  content::WebContents* web_contents =
      browser()->tab_strip_model()->GetActiveWebContents();
  InstallBeforeUnloadHandler(web_contents);

  web_contents->GetController().LoadURL(target, content::Referrer(),
                                        ui::PAGE_TRANSITION_TYPED, {});
  ui_test_utils::WaitForAppModalDialog();
  EXPECT_EQ(url, web_contents->GetLastCommittedURL());
  CancelActiveDialog();
}

IN_PROC_BROWSER_TEST_F(PageExitBrowserTest, BeforeUnloadMasterAllow) {
  const GURL url = embedded_test_server()->GetURL("a.test", "/title1.html");
  const GURL target = embedded_test_server()->GetURL("a.test", "/title2.html");
  SetException(url, ContentSettingsType::BRAVE_USER_CONTROL,
               CONTENT_SETTING_ALLOW);
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));
  content::WebContents* web_contents =
      browser()->tab_strip_model()->GetActiveWebContents();
  InstallBeforeUnloadHandler(web_contents);

  web_contents->GetController().LoadURL(target, content::Referrer(),
                                        ui::PAGE_TRANSITION_TYPED, {});
  ui_test_utils::WaitForAppModalDialog();
  EXPECT_EQ(url, web_contents->GetLastCommittedURL());
  CancelActiveDialog();
}

IN_PROC_BROWSER_TEST_F(PageExitBrowserTest,
                       NormalAllowExceptionsDoNotWeakenIncognito) {
  const GURL url = embedded_test_server()->GetURL("a.test", "/title1.html");
  SetException(url, ContentSettingsType::BRAVE_USER_CONTROL,
               CONTENT_SETTING_ALLOW);
  SetException(url, ContentSettingsType::BRAVE_USER_CONTROL_PAGE_EXIT,
               CONTENT_SETTING_ALLOW);

  Browser* incognito_browser = CreateIncognitoBrowser();
  HostContentSettingsMap* incognito_settings =
      settings(incognito_browser->profile());
  EXPECT_EQ(CONTENT_SETTING_BLOCK,
            incognito_settings->GetContentSetting(
                url, url, ContentSettingsType::BRAVE_USER_CONTROL));
  EXPECT_EQ(CONTENT_SETTING_BLOCK,
            incognito_settings->GetContentSetting(
                url, url, ContentSettingsType::BRAVE_USER_CONTROL_PAGE_EXIT));
}

IN_PROC_BROWSER_TEST_F(PageExitBrowserTest, EmbeddedFrameUsesOutermostSite) {
  const GURL top_url = embedded_test_server()->GetURL(
      "a.test", "/cross_site_iframe_factory.html?a.test(b.test)");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), top_url));
  content::WebContents* web_contents =
      browser()->tab_strip_model()->GetActiveWebContents();
  content::RenderFrameHost* child =
      content::ChildFrameAt(web_contents->GetPrimaryMainFrame(), 0);
  ASSERT_TRUE(child);

  const GURL child_url = child->GetLastCommittedURL();
  SetException(child_url, ContentSettingsType::BRAVE_USER_CONTROL_PAGE_EXIT,
               CONTENT_SETTING_ALLOW);
  EXPECT_TRUE(IsPageExitProtectionEnabled(child));

  SetException(top_url, ContentSettingsType::BRAVE_USER_CONTROL_PAGE_EXIT,
               CONTENT_SETTING_ALLOW);
  EXPECT_FALSE(IsPageExitProtectionEnabled(child));
}

}  // namespace
}  // namespace user_control
