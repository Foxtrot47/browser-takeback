// Copyright (c) 2026 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "brave/browser/user_control/user_control_policy.h"

#include "brave/components/user_control/user_control_policy.h"
#include "chrome/browser/content_settings/host_content_settings_map_factory.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/content_settings/core/common/content_settings_types.h"
#include "content/public/browser/render_frame_host.h"
#include "url/gurl.h"

namespace user_control {

bool IsPageExitProtectionEnabled(content::RenderFrameHost* render_frame_host) {
  if (!render_frame_host) {
    return false;
  }

  content::RenderFrameHost* outermost_main_frame =
      render_frame_host->GetOutermostMainFrame();
  if (!outermost_main_frame) {
    return false;
  }

  const GURL& url = outermost_main_frame->GetLastCommittedURL();
  if (!url.SchemeIsHTTPOrHTTPS()) {
    return false;
  }

  auto* map = HostContentSettingsMapFactory::GetForProfile(
      outermost_main_frame->GetBrowserContext());
  if (!map) {
    return false;
  }

  const ContentSetting master_setting = map->GetContentSetting(
      url, url, ContentSettingsType::BRAVE_USER_CONTROL);
  const ContentSetting category_setting = map->GetContentSetting(
      url, url, ContentSettingsType::BRAVE_USER_CONTROL_PAGE_EXIT);
  return IsProtectionEnabled(master_setting, category_setting);
}

}  // namespace user_control
