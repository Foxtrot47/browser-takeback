// Copyright (c) 2026 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import './category_setting_exceptions.js';
import './settings_category_default_radio_group.js';
import './site_settings_shared.css.js';
import '../settings_page/settings_subpage.js';
import '../settings_shared.css.js';

import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';

import {SettingsViewMixin} from '../settings_page/settings_view_mixin.js';
import {ContentSettingsTypes} from '../site_settings/constants.js';

import {getTemplate} from './site_settings_user_control_page_exit.html.js';

const SiteSettingsUserControlPageExitBase =
    SettingsViewMixin(PolymerElement);

export class SiteSettingsUserControlPageExit extends
    SiteSettingsUserControlPageExitBase {
  static get is() {
    return 'site-settings-user-control-page-exit';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      searchTerm: String,

      contentSettingsTypesEnum_: {
        type: Object,
        value: ContentSettingsTypes,
      },
    };
  }

  declare searchTerm: string;

  override focusBackButton() {
    this.shadowRoot!.querySelector('settings-subpage')!.focusBackButton();
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'site-settings-user-control-page-exit':
        SiteSettingsUserControlPageExit;
  }
}

customElements.define(
    SiteSettingsUserControlPageExit.is, SiteSettingsUserControlPageExit);
