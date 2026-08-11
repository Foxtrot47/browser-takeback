// Copyright (c) 2026 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef BRAVE_COMPONENTS_USER_CONTROL_USER_CONTROL_POLICY_H_
#define BRAVE_COMPONENTS_USER_CONTROL_USER_CONTROL_POLICY_H_

#include <map>

#include "components/content_settings/core/common/content_settings.h"
#include "components/content_settings/core/common/content_settings_types.h"

class GURL;

namespace user_control {

bool IsUserControlContentSetting(ContentSettingsType type);

bool IsProtectionEnabled(ContentSetting master_setting,
                         ContentSetting category_setting);

bool IsProtectionEnabled(
    const std::map<ContentSettingsType, ContentSettingsForOneType>& rules,
    const GURL& outermost_main_frame_url,
    ContentSettingsType category);

}  // namespace user_control

#endif  // BRAVE_COMPONENTS_USER_CONTROL_USER_CONTROL_POLICY_H_
