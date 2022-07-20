/*
 * Copyright 2022 Redpanda Data, Inc.
 *
 * Licensed as a Redpanda Enterprise file under the Redpanda Community
 * License (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 * https://github.com/redpanda-data/redpanda/blob/master/licenses/rcl.md
 */

#pragma once

#include "cloud_storage/base_manifest.h"
#include "cluster/types.h"
#include "json/document.h"
#include "model/metadata.h"

#include <optional>

namespace cloud_storage {

struct topic_manifest_handler;
class topic_manifest final : public base_manifest {
public:
    /// Create manifest for specific ntp
    explicit topic_manifest(
      const cluster::topic_configuration& cfg, model::initial_revision_id rev);

    /// Create empty manifest that supposed to be updated later
    topic_manifest();

    /// Update manifest file from input_stream (remote set)
    ss::future<> update(ss::input_stream<char> is) override;

    /// Serialize manifest object
    ///
    /// \return asynchronous input_stream with the serialized json
    serialized_json_stream serialize() const override;

    /// Manifest object name in S3
    remote_manifest_path get_manifest_path() const override;

    static remote_manifest_path
    get_topic_manifest_path(model::ns ns, model::topic topic);

    /// Serialize manifest object
    ///
    /// \param out output stream that should be used to output the json
    void serialize(std::ostream& out) const;

    manifest_type get_manifest_type() const override {
        return manifest_type::topic;
    };

    model::initial_revision_id get_revision() const noexcept { return _rev; }

    /// Return previous revision id.
    /// If the topic manifest was used for topic recovery the revision id might
    /// be different and recovery process need a way to access previous revision
    /// id.
    model::initial_revision_id get_prev_revision() const noexcept {
        if (_prev_rev == model::initial_revision_id::min()) {
            return _rev;
        }
        return _prev_rev;
    }

    /// Change topic-manifest revision
    void set_revision(model::initial_revision_id id) noexcept {
        _prev_rev = _rev;
        _rev = id;
    }

    std::optional<cluster::topic_configuration> const&
    get_topic_config() const noexcept {
        return _topic_config;
    }

private:
    /// Update manifest content from json document that supposed to be generated
    /// from manifest.json file
    void update(const topic_manifest_handler& handler);

    std::optional<cluster::topic_configuration> _topic_config;
    /// Initial revision of the topic
    model::initial_revision_id _rev;
    /// Initial revision of the topic before recovery.
    /// Topic manifest need to be re-uploaded after successful recovery of the
    /// partition 0. In this case we want to know old revision id to recover
    /// other partitions successfuly.
    model::initial_revision_id _prev_rev{model::initial_revision_id::min()};
};
} // namespace cloud_storage
