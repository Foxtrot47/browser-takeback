// Copyright (c) 2026 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';

import {SettingsViewMixin} from '../settings_page/settings_view_mixin.js';
import {ContentSettingsTypes} from '../site_settings/constants.js';

import {getTemplate} from './site_settings_user_control.html.js';

const SiteSettingsUserControlPageBase = SettingsViewMixin(PolymerElement);

export class SiteSettingsUserControlPage extends
    SiteSettingsUserControlPageBase {
  static get is() {
    return 'site-settings-user-control-page';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      contentSettingsTypesEnum_: {
        type: Object,
        value: ContentSettingsTypes,
      },
    };
  }

  override focusBackButton() {
    this.shadowRoot!.querySelector('settings-subpage')!.focusBackButton();
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'site-settings-user-control-page': SiteSettingsUserControlPage;
  }
}

customElements.define(
    SiteSettingsUserControlPage.is, SiteSettingsUserControlPage);
