// Copyright (c) 2026 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef BRAVE_BROWSER_USER_CONTROL_USER_CONTROL_POLICY_H_
#define BRAVE_BROWSER_USER_CONTROL_USER_CONTROL_POLICY_H_

namespace content {
class RenderFrameHost;
}

namespace user_control {

bool IsPageExitProtectionEnabled(content::RenderFrameHost* render_frame_host);

}  // namespace user_control

#endif  // BRAVE_BROWSER_USER_CONTROL_USER_CONTROL_POLICY_H_
