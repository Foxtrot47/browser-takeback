/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import type { SiteDetailsPermissionElement } from '../site_settings/site_details_permission.js'

import {
  RegisterPolymerPrototypeModification,
  RegisterPolymerTemplateModifications,
  html
} from 'chrome://resources/brave/polymer_overriding.js'
import { loadTimeData } from '../i18n_setup.js'
import {
  ContentSetting,
  ContentSettingsTypes
} from '../site_settings/constants.js'

import 'chrome://resources/brave/leo.bundle.js'

const insertBefore = (element: Element, newElement: Element | Node) => {
  if (!element.parentNode) {
    throw new Error('Element has no parent node - nothing will be inserted')
  }

  element.parentNode.insertBefore(newElement, element)
}

const getUserControlLabelKeys = (category: ContentSettingsTypes) => {
  if (category === ContentSettingsTypes.USER_CONTROL) {
    return {
      allow: 'siteSettingsUserControlAllow',
      block: 'siteSettingsUserControlProtect',
      allowDefault: 'siteSettingsUserControlAllowDefault',
      blockDefault: 'siteSettingsUserControlProtectDefault'
    }
  }
  if (category === ContentSettingsTypes.USER_CONTROL_PAGE_EXIT) {
    return {
      allow: 'siteSettingsUserControlPageExitAllow',
      block: 'siteSettingsUserControlPageExitProtect',
      allowDefault: 'siteSettingsUserControlPageExitAllowDefault',
      blockDefault: 'siteSettingsUserControlPageExitProtectDefault'
    }
  }
  return null
}

RegisterPolymerPrototypeModification({
  'site-details-permission': (prototype) => {
    const originalDefaultSettingString = prototype.defaultSettingString_
    prototype.defaultSettingString_ = function (...args: unknown[]) {
      const [defaultSetting, category] = args as [
        ContentSetting,
        ContentSettingsTypes
      ]
      const keys = getUserControlLabelKeys(category)
      if (keys && defaultSetting === ContentSetting.ALLOW) {
        return this.i18n(keys.allowDefault)
      }
      if (keys && defaultSetting === ContentSetting.BLOCK) {
        return this.i18n(keys.blockDefault)
      }
      return originalDefaultSettingString.apply(this, args)
    }
    prototype.braveAllowSettingString_ = function (
        category: ContentSettingsTypes) {
      const keys = getUserControlLabelKeys(category)
      return this.i18n(keys?.allow ?? 'siteSettingsActionAllow')
    }
    prototype.braveBlockSettingString_ = function (
        category: ContentSettingsTypes) {
      const keys = getUserControlLabelKeys(category)
      return this.i18n(keys?.block ?? 'siteSettingsActionBlock')
    }
  }
})

RegisterPolymerTemplateModifications({
  'site-details-permission': (templateContent: HTMLTemplateElement) => {
    const allowOption = templateContent.querySelector('#allow')
    const blockOption = templateContent.querySelector('#block')
    if (!allowOption || !blockOption) {
      console.error('[Settings] Couldn\'t customize site permission labels')
      return
    }
    allowOption.textContent = '[[braveAllowSettingString_(category)]]'
    blockOption.textContent = '[[braveBlockSettingString_(category)]]'
  },
  'site-details': (templateContent: HTMLTemplateElement) => {
    // Add top-padding to subpage
    templateContent.prepend(
      html`<style>#usage { padding-top: var(--leo-spacing-l); }</style>`)

    if (!loadTimeData.getBoolean('isIdleDetectionFeatureEnabled')) {
      const idleDetectionItem =
        templateContent.querySelector<SiteDetailsPermissionElement>(
          '[category="[[contentSettingsTypesEnum_.IDLE_DETECTION]]"]')
      if (!idleDetectionItem) {
        console.error('[Settings] Couldn\'t find idle detection item')
      } else {
        idleDetectionItem.hidden = true
      }
    }
    const adsItem =
      templateContent.querySelector<SiteDetailsPermissionElement>(
        '[category="[[contentSettingsTypesEnum_.ADS]]"]')
    if (!adsItem) {
      console.error('[Settings] Couldn\'t find ads item')
    } else {
      adsItem.hidden = true
    }
    const firstPermissionItem = templateContent.querySelector(
      'div.list-frame > site-details-permission:nth-child(1)')
    if (!firstPermissionItem) {
      console.error('[Settings] Couldn\'t find first permission item')
    } else {
      insertBefore(firstPermissionItem, html`<site-details-permission
           category="[[contentSettingsTypesEnum_.USER_CONTROL]]"
           icon="shield-done">
         </site-details-permission>`)
      insertBefore(firstPermissionItem, html`<site-details-permission
           category="[[contentSettingsTypesEnum_.USER_CONTROL_PAGE_EXIT]]"
           icon="shield-done">
         </site-details-permission>`)
      let curChild = 1
      const userControlSettings = templateContent.querySelector(
        `div.list-frame > site-details-permission:nth-child(${curChild})`)
      if (!userControlSettings) {
        console.error('[Settings] Couldn\'t find user-control settings')
      } else {
        userControlSettings.setAttribute(
          'label', loadTimeData.getString('siteSettingsUserControl'))
      }
      curChild++
      const userControlPageExitSettings = templateContent.querySelector(
        `div.list-frame > site-details-permission:nth-child(${curChild})`)
      if (!userControlPageExitSettings) {
        console.error('[Settings] Couldn\'t find page-exit settings')
      } else {
        userControlPageExitSettings.setAttribute(
          'label', loadTimeData.getString('siteSettingsUserControlPageExit'))
      }
      curChild++
      insertBefore(firstPermissionItem, html`<site-details-permission
           category="[[contentSettingsTypesEnum_.AUTOPLAY]]"
           icon="autoplay-on">
         </site-details-permission>`)
      const autoplaySettings = templateContent.querySelector(
        `div.list-frame > site-details-permission:nth-child(${curChild})`)
      if (!autoplaySettings) {
        console.error('[Settings] Couldn\'t find autoplay settings')
      }
      else {
        autoplaySettings.setAttribute(
          'label', loadTimeData.getString('siteSettingsAutoplay'))
      }
      curChild++
      // Google Sign-In feature
      const isGoogleSignInFeatureEnabled =
        loadTimeData.getBoolean('isGoogleSignInFeatureEnabled')
      if (isGoogleSignInFeatureEnabled) {
        insertBefore(firstPermissionItem, html`<site-details-permission
             category="[[contentSettingsTypesEnum_.GOOGLE_SIGN_IN]]"
             icon="user">
           </site-details-permission>`)
        const googleSignInSettings = templateContent.querySelector(
          `div.list-frame > site-details-permission:nth-child(${curChild})`)
        if (!googleSignInSettings) {
          console.error('[Settings] Couldn\'t find Google signin settings')
        }
        else {
          googleSignInSettings.setAttribute(
            'label', loadTimeData.getString('siteSettingsGoogleSignIn'))
        }
        curChild++
      }
      // <if expr="enable_ai_chat">
      // AI Chat feature
      const isOpenAIChatFromBraveSearchEnabled =
        loadTimeData.getBoolean('isOpenAIChatFromBraveSearchEnabled')
      if (isOpenAIChatFromBraveSearchEnabled) {
        insertBefore(firstPermissionItem, html`<site-details-permission
             category="[[contentSettingsTypesEnum_.BRAVE_OPEN_AI_CHAT]]"
             icon="user">
           </site-details-permission>`)
        const braveAIChatSettings = templateContent.querySelector(
          `div.list-frame > site-details-permission:nth-child(${curChild})`)
        if (!braveAIChatSettings) {
          console.error('[Settings] Couldn\'t find Brave AI chat settings')
        } else {
          braveAIChatSettings.setAttribute(
            'label', loadTimeData.getString('siteSettingsBraveOpenAIChat'))
        }
        curChild++
      }
      // </if>

      // <if expr="enable_brave_wallet">
      const isCardanoDappSupportFeatureEnabled =
          loadTimeData.getBoolean('isCardanoDappSupportFeatureEnabled')
      const isBraveWalletAllowed =
          loadTimeData.getBoolean('isBraveWalletAllowed')
      if (isBraveWalletAllowed) {
        insertBefore(firstPermissionItem, html`<site-details-permission
             category="[[contentSettingsTypesEnum_.ETHEREUM]]"
             icon="ethereum-on">
           </site-details-permission>`)
        const ethereumSettings = templateContent.querySelector(
          `div.list-frame > site-details-permission:nth-child(${curChild})`)
        if (!ethereumSettings) {
          console.error('[Settings] Couldn\'t find Ethereum settings')
        } else {
          ethereumSettings.setAttribute(
            'label', loadTimeData.getString('siteSettingsEthereum'))
        }
        curChild++
        insertBefore(firstPermissionItem, html`<site-details-permission
             category="[[contentSettingsTypesEnum_.SOLANA]]"
             icon="solana-on">
           </site-details-permission>`)
        const solanaSettings = templateContent.querySelector(
          `div.list-frame > site-details-permission:nth-child(${curChild})`)
        if (!solanaSettings) {
          console.error('[Settings] Couldn\'t find Solana settings')
        } else {
          solanaSettings.setAttribute(
            'label', loadTimeData.getString('siteSettingsSolana'))
        }
        if (isCardanoDappSupportFeatureEnabled) {
          curChild++
          insertBefore(firstPermissionItem, html`<site-details-permission
               category="[[contentSettingsTypesEnum_.CARDANO]]"
               icon="cardano-on">
             </site-details-permission>`)
          const cardanoSettings = templateContent.querySelector(
            `div.list-frame > site-details-permission:nth-child(${curChild})`)
          if (!cardanoSettings) {
            console.error('[Settings] Couldn\'t find Cardano settings')
          } else {
            cardanoSettings.setAttribute(
              'label', loadTimeData.getString('siteSettingsCardano'))
          }
        }
      }
      // </if>

      const adPersonalization =
        templateContent.querySelector('#adPersonalization')
      if (!adPersonalization) {
        console.error(
          '[Settings] Could not find adPersonalization element to hide')
      } else {
        adPersonalization.remove()
      }
    }

    // In Chromium, the VR and AR icons are the same but we want to have
    // separate ones.
    templateContent.
      querySelector('site-details-permission[icon="settings:vr-headset"]')?.
      setAttribute('icon', 'smartphone-hand')
  }
})
