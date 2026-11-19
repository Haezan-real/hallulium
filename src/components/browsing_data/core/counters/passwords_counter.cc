// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Hallulium: no-op passwords counter (password_manager cut).

#include "components/browsing_data/core/counters/passwords_counter.h"

#include <utility>

#include "base/functional/bind.h"
#include "components/browsing_data/core/pref_names.h"
#include "components/sync/service/sync_service.h"

namespace browsing_data {

PasswordsCounter::PasswordsResult::PasswordsResult(
    const BrowsingDataCounter* source,
    ResultInt profile_passwords,
    ResultInt account_passwords,
    bool sync_enabled,
    std::vector<std::string> domain_examples,
    std::vector<std::string> account_domain_examples)
    : SyncResult(source, profile_passwords, sync_enabled),
      account_passwords_(account_passwords),
      domain_examples_(std::move(domain_examples)),
      account_domain_examples_(std::move(account_domain_examples)) {}

PasswordsCounter::PasswordsResult::~PasswordsResult() = default;

PasswordsCounter::PasswordsCounter(PrefService* pref_service,
                                   syncer::SyncService* sync_service)
    : sync_tracker_(this, sync_service), pref_service_(pref_service) {}

PasswordsCounter::~PasswordsCounter() = default;

int PasswordsCounter::num_passwords() {
  return 0;
}

int PasswordsCounter::num_account_passwords() {
  return 0;
}

const std::vector<std::string>& PasswordsCounter::domain_examples() {
  return empty_domains_;
}

const std::vector<std::string>& PasswordsCounter::account_domain_examples() {
  return empty_domains_;
}

void PasswordsCounter::OnInitialized() {
  sync_tracker_.OnInitialized(base::BindRepeating(
      [](const syncer::SyncService*) { return false; }));
}

const char* PasswordsCounter::GetPrefName() const {
  return prefs::kDeletePasswords;
}

void PasswordsCounter::Count() {
  ReportResult(MakeResult());
}

void PasswordsCounter::OnPasswordsFetchDone() {}

std::unique_ptr<PasswordsCounter::PasswordsResult>
PasswordsCounter::MakeResult() {
  return std::make_unique<PasswordsCounter::PasswordsResult>(
      this, 0, 0, is_sync_active(), std::vector<std::string>(),
      std::vector<std::string>());
}

}  // namespace browsing_data
