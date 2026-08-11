// Copyright (c) 2026 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "brave/components/user_control/user_control_policy.h"

#include <optional>

#include "base/containers/map_util.h"
#include "url/gurl.h"

namespace user_control {
namespace {

std::optional<ContentSetting> GetSetting(
    const std::map<ContentSettingsType, ContentSettingsForOneType>& rules,
    ContentSettingsType type,
    const GURL& outermost_main_frame_url) {
  const auto* rules_for_type = base::FindOrNull(rules, type);
  if (!rules_for_type) {
    return std::nullopt;
  }

  for (const auto& rule : *rules_for_type) {
    if (rule.primary_pattern.Matches(outermost_main_frame_url) &&
        rule.secondary_pattern.Matches(outermost_main_frame_url)) {
      return rule.GetContentSetting();
    }
  }
  return std::nullopt;
}

}  // namespace

bool IsUserControlContentSetting(ContentSettingsType type) {
  return type == ContentSettingsType::BRAVE_USER_CONTROL ||
         type == ContentSettingsType::BRAVE_USER_CONTROL_PAGE_EXIT;
}

bool IsProtectionEnabled(ContentSetting master_setting,
                         ContentSetting category_setting) {
  return master_setting != CONTENT_SETTING_ALLOW &&
         category_setting != CONTENT_SETTING_ALLOW;
}

bool IsProtectionEnabled(
    const std::map<ContentSettingsType, ContentSettingsForOneType>& rules,
    const GURL& outermost_main_frame_url,
    ContentSettingsType category) {
  if (!outermost_main_frame_url.SchemeIsHTTPOrHTTPS() ||
      category == ContentSettingsType::BRAVE_USER_CONTROL ||
      !IsUserControlContentSetting(category)) {
    return false;
  }

  const auto master_setting =
      GetSetting(rules, ContentSettingsType::BRAVE_USER_CONTROL,
                 outermost_main_frame_url);
  const auto category_setting =
      GetSetting(rules, category, outermost_main_frame_url);
  return master_setting && category_setting &&
         IsProtectionEnabled(*master_setting, *category_setting);
}

}  // namespace user_control
