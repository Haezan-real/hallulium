// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Hallulium: password-store-backed counter stubbed out (password_manager cut).

#ifndef COMPONENTS_BROWSING_DATA_CORE_COUNTERS_PASSWORDS_COUNTER_H_
#define COMPONENTS_BROWSING_DATA_CORE_COUNTERS_PASSWORDS_COUNTER_H_

#include <memory>
#include <string>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "components/browsing_data/core/counters/browsing_data_counter.h"
#include "components/browsing_data/core/counters/sync_tracker.h"

class PrefService;

namespace syncer {
class SyncService;
}

namespace browsing_data {

class PasswordsCounter : public browsing_data::BrowsingDataCounter {
 public:
  class PasswordsResult : public SyncResult {
   public:
    PasswordsResult(const BrowsingDataCounter* source,
                    ResultInt profile_passwords,
                    ResultInt account_passwords,
                    bool sync_enabled,
                    std::vector<std::string> domain_examples,
                    std::vector<std::string> account_domain_examples);

    PasswordsResult(const PasswordsResult&) = delete;
    PasswordsResult& operator=(const PasswordsResult&) = delete;

    ~PasswordsResult() override;

    ResultInt account_passwords() const { return account_passwords_; }

    const std::vector<std::string>& domain_examples() const {
      return domain_examples_;
    }

    const std::vector<std::string>& account_domain_examples() const {
      return account_domain_examples_;
    }

   private:
    ResultInt account_passwords_ = 0;
    std::vector<std::string> domain_examples_;
    std::vector<std::string> account_domain_examples_;
  };

  PasswordsCounter(PrefService* pref_service,
                   syncer::SyncService* sync_service);
  ~PasswordsCounter() override;

  const char* GetPrefName() const override;

 protected:
  virtual void OnPasswordsFetchDone();
  virtual std::unique_ptr<PasswordsResult> MakeResult();
  void Count() override;

  bool is_sync_active() { return sync_tracker_.IsSyncActive(); }
  int num_passwords();
  int num_account_passwords();
  const std::vector<std::string>& domain_examples();
  const std::vector<std::string>& account_domain_examples();

 private:
  void OnInitialized() override;

  SyncTracker sync_tracker_;
  const raw_ptr<PrefService> pref_service_;
  std::vector<std::string> empty_domains_;

  base::WeakPtrFactory<PasswordsCounter> weak_ptr_factory_{this};
};

}  // namespace browsing_data

#endif  // COMPONENTS_BROWSING_DATA_CORE_COUNTERS_PASSWORDS_COUNTER_H_
