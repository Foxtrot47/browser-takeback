/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/core/frame/dom_window.h"

#include "components/content_settings/core/common/content_settings_types.h"
#include "third_party/blink/public/platform/web_content_settings_client.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"

#define BRAVE_DOM_WINDOW_CLOSE                                      \
  if (LocalFrame* incumbent_frame = incumbent_window->GetFrame()) { \
    if (WebContentSettingsClient* settings_client =                 \
            incumbent_frame->GetContentSettingsClient();            \
        settings_client &&                                          \
        settings_client->IsUserControlProtectionEnabled(            \
            ContentSettingsType::BRAVE_USER_CONTROL_PAGE_EXIT)) {   \
      return;                                                       \
    }                                                               \
  }

#include <third_party/blink/renderer/core/frame/dom_window.cc>
#undef BRAVE_DOM_WINDOW_CLOSE

namespace blink {

LocalFrame* DOMWindow::GetDisconnectedFrame() const {
  // IncumbentDOMWindow is safe to call only when an active v8 context is
  // present.
  if (auto* isolate = v8::Isolate::TryGetCurrent();
      !isolate || !isolate->InContext()) {
    return nullptr;
  }

  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  LocalDOMWindow* accessing_window = IncumbentDOMWindow(isolate);
  LocalFrame* accessing_frame = accessing_window->GetFrame();
  return accessing_frame;
}

}  // namespace blink
