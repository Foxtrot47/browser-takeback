// Copyright (c) 2026 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "brave/components/user_control/user_control_policy.h"

#include "components/content_settings/core/common/content_settings_pattern.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace user_control {
namespace {

ContentSettingPatternSource MakeRule(const std::string& primary_pattern,
                                     ContentSetting setting) {
  return ContentSettingPatternSource(
      ContentSettingsPattern::FromString(primary_pattern),
      ContentSettingsPattern::Wildcard(), base::Value(setting),
      content_settings::mojom::ProviderType::kDefaultProvider, false);
}

TEST(UserControlPolicyTest, MasterAndCategoryMustBothProtect) {
  EXPECT_TRUE(
      IsProtectionEnabled(CONTENT_SETTING_BLOCK, CONTENT_SETTING_BLOCK));
  EXPECT_FALSE(
      IsProtectionEnabled(CONTENT_SETTING_ALLOW, CONTENT_SETTING_BLOCK));
  EXPECT_FALSE(
      IsProtectionEnabled(CONTENT_SETTING_BLOCK, CONTENT_SETTING_ALLOW));
  EXPECT_FALSE(
      IsProtectionEnabled(CONTENT_SETTING_ALLOW, CONTENT_SETTING_ALLOW));
}

TEST(UserControlPolicyTest, UsesOutermostSiteRules) {
  const GURL protected_url("https://protected.test/");
  const GURL allowed_url("https://allowed.test/");
  std::map<ContentSettingsType, ContentSettingsForOneType> rules;
  rules[ContentSettingsType::BRAVE_USER_CONTROL] = {
      MakeRule("https://allowed.test", CONTENT_SETTING_ALLOW),
      MakeRule("*", CONTENT_SETTING_BLOCK),
  };
  rules[ContentSettingsType::BRAVE_USER_CONTROL_PAGE_EXIT] = {
      MakeRule("*", CONTENT_SETTING_BLOCK),
  };

  EXPECT_TRUE(IsProtectionEnabled(
      rules, protected_url, ContentSettingsType::BRAVE_USER_CONTROL_PAGE_EXIT));
  EXPECT_FALSE(IsProtectionEnabled(
      rules, allowed_url, ContentSettingsType::BRAVE_USER_CONTROL_PAGE_EXIT));
}

TEST(UserControlPolicyTest, AppliesToWebAndLocalPagesOnly) {
  EXPECT_TRUE(IsUserControlProtectionApplicable(GURL("https://example.test/")));
  EXPECT_TRUE(IsUserControlProtectionApplicable(GURL("http://example.test/")));
  EXPECT_TRUE(IsUserControlProtectionApplicable(GURL("file:///C:/test.html")));
  EXPECT_FALSE(IsUserControlProtectionApplicable(GURL("chrome://settings/")));
  EXPECT_FALSE(IsUserControlProtectionApplicable(GURL("about:blank")));
}

TEST(UserControlPolicyTest, MissingRulesPreserveBehavior) {
  std::map<ContentSettingsType, ContentSettingsForOneType> rules;
  rules[ContentSettingsType::BRAVE_USER_CONTROL] = {
      MakeRule("*", CONTENT_SETTING_BLOCK),
  };

  EXPECT_FALSE(
      IsProtectionEnabled(rules, GURL("https://example.test/"),
                          ContentSettingsType::BRAVE_USER_CONTROL_PAGE_EXIT));
}

TEST(UserControlPolicyTest, ProtectsLocalFilesWithDefaultRules) {
  std::map<ContentSettingsType, ContentSettingsForOneType> rules;
  rules[ContentSettingsType::BRAVE_USER_CONTROL] = {
      MakeRule("*", CONTENT_SETTING_BLOCK),
  };
  rules[ContentSettingsType::BRAVE_USER_CONTROL_PAGE_EXIT] = {
      MakeRule("*", CONTENT_SETTING_BLOCK),
  };

  EXPECT_TRUE(
      IsProtectionEnabled(rules, GURL("file:///C:/test.html"),
                          ContentSettingsType::BRAVE_USER_CONTROL_PAGE_EXIT));
  EXPECT_FALSE(
      IsProtectionEnabled(rules, GURL("chrome://settings/"),
                          ContentSettingsType::BRAVE_USER_CONTROL_PAGE_EXIT));
}

}  // namespace
}  // namespace user_control
