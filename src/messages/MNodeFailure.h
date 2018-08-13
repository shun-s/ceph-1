// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2004-2006 Song Shun <song.shun3@zte.com.cn>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 *
 */


#ifndef CEPH_MNODEFAILURE_H
#define CEPH_MNODEFAILURE_H

#include "messages/PaxosServiceMessage.h"


class MNodeFailure : public PaxosServiceMessage {

  static const int HEAD_VERSION = 1;
  static const int COMPAT_VERSION = 1;

 public:
  enum {
    FLAG_ALIVE = 0,      // use this on its own to mark as "I'm still alive"
    FLAG_FAILED = 1,     // if set, failure; if not, recovery
  };

  uuid_d fsid;
  int target_crush_id;
  __u8 flags = 0;
  epoch_t epoch = 0;
  double failed_for = 0;  // known to be failed since at least this long

  MNodeFailure() : PaxosServiceMessage(MSG_NODE_FAILURE, 0, HEAD_VERSION) { }
  MNodeFailure(const uuid_d &fs, int id, double duration, epoch_t e)
    : PaxosServiceMessage(MSG_NODE_FAILURE, e, HEAD_VERSION, COMPAT_VERSION),
      fsid(fs), target_crush_id(id),
      flags(FLAG_FAILED),
      epoch(e), failed_for(duration) { }
  MNodeFailure(const uuid_d &fs, int id, double duration,
              epoch_t e, __u8 extra_flags)
    : PaxosServiceMessage(MSG_NODE_FAILURE, e, HEAD_VERSION, COMPAT_VERSION),
      fsid(fs), target_crush_id(id),
      flags(extra_flags),
      epoch(e), failed_for(duration) { }
private:
  ~MNodeFailure() override {}

public:
  int get_target() { return target_crush_id; }
  bool if_node_failed() const {
    return flags & FLAG_FAILED;
  }
  epoch_t get_epoch() const { return epoch; }

  void decode_payload() override {
    bufferlist::iterator p = payload.begin();
    paxos_decode(p);
    ::decode(fsid, p);
    ::decode(target_crush_id, p);
    ::decode(epoch, p);
    ::decode(flags, p);
    ::decode(failed_for, p);
  }

  void encode_payload(uint64_t features) override {
    paxos_encode();
    ::encode(fsid, payload);
    ::encode(target_crush_id, payload, features);
    ::encode(epoch, payload);
    ::encode(flags, payload);
    ::encode(failed_for, payload);
  }

  const char *get_type_name() const override { return "node_failure"; }
  void print(ostream& out) const override {
    out << "node_failure("
	<< (if_node_failed() ? "failed " : "alive ")
	<< target_crush_id << " for " << failed_for << "sec e" << epoch
	<< " v" << version << ")";
  }
};

#endif
