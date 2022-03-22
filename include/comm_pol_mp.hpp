//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018-2021, Lawrence Livermore National Security, LLC.
//
// Produced at the Lawrence Livermore National Laboratory
//
// LLNL-CODE-758885
//
// All rights reserved.
//
// This file is part of Comb.
//
// For details, see https://github.com/LLNL/Comb
// Please also see the LICENSE file for MIT license.
//////////////////////////////////////////////////////////////////////////////

#ifndef _COMM_POL_MP_HPP
#define _COMM_POL_MP_HPP

#include "config.hpp"

#ifdef COMB_ENABLE_MP

#include <exception>
#include <stdexcept>
#include <algorithm>
#include <unordered_set>
#include <map>

#include "exec.hpp"
#include "comm_utils_mp.hpp"
#include "MessageBase.hpp"
#include "ExecContext.hpp"

namespace detail
{
namespace mp
{
struct Request
{
	mp_request_t request;
	mp_reg_t reg;
};
}
}

struct mp_pol {
  // static const bool async = false;
  static const bool mock = false;
  // compile mpi_type packing/unpacking tests for this comm policy
  static const bool use_mpi_type = false;
  static const bool persistent = true;
  static const char* get_name() { return "mp"; }
  using send_request_type = detail::mp::Request;
  using recv_request_type = detail::mp::Request;
  using send_status_type = int;
  using recv_status_type = int;
};

template < >
struct CommContext<mp_pol> : CudaContext
{
  using base = CudaContext;

  using pol = mp_pol;

  using send_request_type = typename pol::send_request_type;
  using recv_request_type = typename pol::recv_request_type;
  using send_status_type = typename pol::send_status_type;
  using recv_status_type = typename pol::recv_status_type;

  bool ownmp;

  CommContext()
    : base()
    , ownmp(false)
  { }

  CommContext(base const& b)
    : base(b)
    , ownmp(false)
  { }

  CommContext(CommContext const& a_, MPI_Comm comm_)
    : base(a_)
    , ownmp(true)
  {
    detail::mp::init(comm_);
  }

  ~CommContext()
  {
    if (ownmp) {
      detail::mp::finalize(); ownmp = false;
    }
  }

  void ensure_waitable()
  {

  }

  template < typename context >
  void waitOn(context& con)
  {
    con.ensure_waitable();
    base::waitOn(con);
  }

  send_request_type send_request_null() { return detail::mp::Request{}; }
  recv_request_type recv_request_null() { return detail::mp::Request{}; }
  send_status_type send_status_null() { return 0; }
  recv_status_type recv_status_null() { return 0; }


  void connect_ranks(std::vector<int> const& send_ranks,
                     std::vector<int> const& recv_ranks)
  {
    COMB::ignore_unused(send_ranks, recv_ranks);
  }

  void disconnect_ranks(std::vector<int> const& send_ranks,
                        std::vector<int> const& recv_ranks)
  {
    COMB::ignore_unused(send_ranks, recv_ranks);
  }

  void setup_mempool(COMB::Allocator& many_aloc,
                     COMB::Allocator& few_aloc)
  {
	  
  }

  void teardown_mempool()
  {
	  
  }

};


namespace detail {

template < >
struct Message<MessageBase::Kind::send, mp_pol>
  : MessageInterface<MessageBase::Kind::send, mp_pol>
{
  using base = MessageInterface<MessageBase::Kind::send, mp_pol>;

  using policy_comm = typename base::policy_comm;
  using communicator_type = typename base::communicator_type;
  using request_type      = typename base::request_type;
  using status_type       = typename base::status_type;

  // use the base class constructor
  using base::base;


  static int test_send_any(communicator_type& con_comm,
                           int count, request_type* requests,
                           status_type* statuses)
  {
	  return -1;
  }

  static int wait_send_any(communicator_type& con_comm,
                           int count, request_type* requests,
                           status_type* statuses)
  {
	  return -1;
  }

  static int test_send_some(communicator_type& con_comm,
                            int count, request_type* requests,
                            int* indices, status_type* statuses)
  {
	  return -1;
  }

  static int wait_send_some(communicator_type& con_comm,
                            int count, request_type* requests,
                            int* indices, status_type* statuses)
  {
	  return -1;
  }

  static bool test_send_all(communicator_type& con_comm,
                            int count, request_type* requests,
                            status_type* statuses)
  {
	  return false;
  }

  static void wait_send_all(communicator_type& con_comm,
                            int count, request_type* requests,
                            status_type* statuses)
  {
	  for(IdxT i = 0; i < count; i++)
	  {
		  detail::mp::wait(&requests[i].request);
	  }
  }

};


template < >
struct Message<MessageBase::Kind::recv, mp_pol>
  : MessageInterface<MessageBase::Kind::recv, mp_pol>
{
  using base = MessageInterface<MessageBase::Kind::recv, mp_pol>;

  using policy_comm = typename base::policy_comm;
  using communicator_type = typename base::communicator_type;
  using request_type      = typename base::request_type;
  using status_type       = typename base::status_type;

  // use the base class constructor
  using base::base;


  static int test_recv_any(communicator_type& con_comm,
                           int count, request_type* requests,
                           status_type* statuses)
  {
	  return -1;
  }

  static int wait_recv_any(communicator_type& con_comm,
                           int count, request_type* requests,
                           status_type* statuses)
  {
	  return -1;
  }

  static int test_recv_some(communicator_type& con_comm,
                            int count, request_type* requests,
                            int* indices, status_type* statuses)
  {
	  return -1;
  }

  static int wait_recv_some(communicator_type& con_comm,
                            int count, request_type* requests,
                            int* indices, status_type* statuses)
  {
	  return -1;
  }

  static bool test_recv_all(communicator_type& con_comm,
                            int count, request_type* requests,
                            status_type* statuses)
  {
	  return false;
  }

  static void wait_recv_all(communicator_type& con_comm,
                            int count, request_type* requests,
                            status_type* statuses)
  {
	  for(IdxT i = 0; i < count; i++)
	  {
		  detail::mp::wait(&requests[i].request);
	  }
  }

};


template < typename exec_policy >
struct MessageGroup<MessageBase::Kind::send, mp_pol, exec_policy>
  : detail::MessageGroupInterface<MessageBase::Kind::send, mp_pol, exec_policy>
{
  using base = detail::MessageGroupInterface<MessageBase::Kind::send, mp_pol, exec_policy>;

  using policy_comm       = typename base::policy_comm;
  using communicator_type = typename base::communicator_type;
  using message_type      = typename base::message_type;
  using request_type      = typename base::request_type;
  using status_type       = typename base::status_type;

  using message_item_type = typename base::message_item_type;
  using context_type      = typename base::context_type;
  using event_type        = typename base::event_type;
  using group_type        = typename base::group_type;
  using component_type    = typename base::component_type;

  // use the base class constructor
  using base::base;


  void finalize()
  {
    // call base finalize
    base::finalize();
  }

  void setup(context_type& con, communicator_type& con_comm, message_type** msgs, IdxT len, request_type* requests)
  {
    for(IdxT i = 0; i < len; i++)
	{
		message_type* msg = msgs[i];
		IdxT msg_nbytes = msg->nbytes() * this->m_variables.size();
		msg->buf = this->m_aloc.allocate(msg_nbytes);
		assert(msg->buf != nullptr);
		char* buf = static_cast<char*>(msg->buf);
		LOGPRINTF("Registering %zu bytes for send\n", msg_nbytes);
		requests[i].reg = detail::mp::register_(buf, msg_nbytes);
	}
  }

  void cleanup(communicator_type& con_comm, message_type** msgs, IdxT len, request_type* requests)
  {
    for(IdxT i = 0; i < len; i++)
	{
		message_type* msg = msgs[i];
		this->m_aloc.deallocate(msg->buf);
		
		detail::mp::deregister(requests[i].reg);
	}
  }

  void allocate(context_type& con, communicator_type& con_comm, message_type** msgs, IdxT len, detail::Async async)
  {
	  if(len <= 0) return;
	  
	  if(comb_allow_pack_loop_fusion())
	  {
		  this->m_fuser.allocate(con, this->m_variables, this->m_items.size());
	  }
  }

  void pack(context_type& con, communicator_type& con_comm, message_type** msgs, IdxT len, detail::Async async)
  {
    COMB::ignore_unused(con_comm);
    if (len <= 0) return;
    con.start_group(this->m_groups[len-1]);
    for (IdxT i = 0; i < len; ++i) {
      const message_type* msg = msgs[i];
      char* buf = static_cast<char*>(msg->buf);
      assert(buf != nullptr);
      this->m_contexts[msg->idx].start_component(this->m_groups[len-1], this->m_components[msg->idx]);
      for (const MessageItemBase* msg_item : msg->message_items) {
        const message_item_type* item = static_cast<const message_item_type*>(msg_item);
        const IdxT len = item->size;
        const IdxT nbytes = item->nbytes;
        LidxT const* indices = item->indices;
        for (DataT const* src : this->m_variables) {
          // LOGPRINTF("%p pack %p = %p[%p] len %d\n", this, buf, src, indices, len);
          this->m_contexts[msg->idx].for_all(len, make_copy_idxr_idxr(src, detail::indexer_list_i{indices},
                                             static_cast<DataT*>(static_cast<void*>(buf)), detail::indexer_i{}));
          buf += nbytes;
        }
      }
      if (async == detail::Async::no) {
        this->m_contexts[msg->idx].finish_component(this->m_groups[len-1], this->m_components[msg->idx]);
      } else {
        this->m_contexts[msg->idx].finish_component_recordEvent(this->m_groups[len-1], this->m_components[msg->idx], this->m_events[msg->idx]);
      }
    }
    con.finish_group(this->m_groups[len-1]);
  }

  IdxT wait_pack_complete(context_type& con, communicator_type& con_comm, message_type** msgs, IdxT len, detail::Async async)
  {
    // LOGPRINTF("wait_pack_complete\n");

    // mp isends use message packing context and don't need synchronization
    COMB::ignore_unused(con, con_comm, msgs, async);
    return len;
  }

  static void start_Isends(context_type& con, communicator_type& con_comm)
  {
	  
  }

  void Isend(CudaContext& con, communicator_type& con_comm, message_type** msgs, IdxT len, detail::Async async, request_type* requests)
  {
	  if(len <= 0) return;
	  
	  for(IdxT i = 0; i < len; i++)
	  {
		  message_type* msg = msgs[i];
		  IdxT msg_nbytes = msg->nbytes() * this->m_variables.size();
		  assert(msg->buf != nullptr);
		  detail::mp::isend_on_stream(msg->buf, msg_nbytes, msg->partner_rank, &requests[i].reg, &requests[i].request, con.stream_launch());
	  }
  }
  
  void Isend(CPUContext& con, communicator_type& con_comm, message_type** msgs, IdxT len, detail::Async async, request_type* requests)
  {
	  LOGPRINTF("Entered empty Isend\n");
  }

  static void finish_Isends(context_type& con, communicator_type& con_comm)
  {
	  
  }

  void deallocate(context_type& con, communicator_type& con_comm, message_type** msgs, IdxT len, detail::Async async)
  {
	  if(len <= 0) return;
	  
	  if(comb_allow_pack_loop_fusion())
	  {
		  this->m_fuser.deallocate(con);
	  }
  }
};

template < typename exec_policy >
struct MessageGroup<MessageBase::Kind::recv, mp_pol, exec_policy>
  : detail::MessageGroupInterface<MessageBase::Kind::recv, mp_pol, exec_policy>
{
  using base = detail::MessageGroupInterface<MessageBase::Kind::recv, mp_pol, exec_policy>;

  using policy_comm       = typename base::policy_comm;
  using communicator_type = typename base::communicator_type;
  using message_type      = typename base::message_type;
  using request_type      = typename base::request_type;
  using status_type       = typename base::status_type;

  using message_item_type = typename base::message_item_type;
  using context_type      = typename base::context_type;
  using event_type        = typename base::event_type;
  using group_type        = typename base::group_type;
  using component_type    = typename base::component_type;

  // use the base class constructor
  using base::base;


  void finalize()
  {
    // call base finalize
    base::finalize();
  }

  void setup(context_type& con, communicator_type& con_comm, message_type** msgs, IdxT len, request_type* requests)
  {
    for(IdxT i = 0; i < len; i++)
	{
		message_type* msg = msgs[i];
		IdxT msg_nbytes = msg->nbytes() * this->m_variables.size();
		msg->buf = this->m_aloc.allocate(msg_nbytes);
		assert(msg->buf != nullptr);
		char* buf = static_cast<char*>(msg->buf);
		LOGPRINTF("Registering %zu bytes for recv %zu %p\n", msg_nbytes, i, &requests[i]);
		requests[i].reg = detail::mp::register_(buf, msg_nbytes);
	}
  }

  void cleanup(communicator_type& con_comm, message_type** msgs, IdxT len, request_type* requests)
  {
    for(IdxT i = 0; i < len; i++)
	{
		message_type* msg = msgs[i];
		this->m_aloc.deallocate(msg->buf);
		
		detail::mp::deregister(requests[i].reg);
	}
  }

  void allocate(context_type& con, communicator_type& con_comm, message_type** msgs, IdxT len, detail::Async async)
  {
	  if(len <= 0) return;
	  
	  if(comb_allow_pack_loop_fusion())
	  {
		  this->m_fuser.allocate(con, this->m_variables, this->m_items.size());
	  }
  }

  void Irecv(context_type& con, communicator_type& con_comm, message_type** msgs, IdxT len, detail::Async async, request_type* requests)
  {
	  if(len <= 0) return;
	  
	  for(IdxT i = 0; i < len; i++)
	  {
		  message_type* msg = msgs[i];
		  IdxT msg_nbytes = msg->nbytes() * this->m_variables.size();
		  assert(msg->buf != nullptr);
		  detail::mp::irecv(msg->buf, msg_nbytes, msg->partner_rank, &requests[i].reg, &requests[i].request);
	  }
  }

  void unpack(context_type& con, communicator_type& con_comm, message_type** msgs, IdxT len, detail::Async async)
  {
    COMB::ignore_unused(con_comm, async);
    if (len <= 0) return;
    con.start_group(this->m_groups[len-1]);
    for (IdxT i = 0; i < len; ++i) {
      const message_type* msg = msgs[i];
      char* buf = static_cast<char*>(msg->buf);
      assert(buf != nullptr);
      this->m_contexts[msg->idx].start_component(this->m_groups[len-1], this->m_components[msg->idx]);
      for (const MessageItemBase* msg_item : msg->message_items) {
        const message_item_type* item = static_cast<const message_item_type*>(msg_item);
        const IdxT len = item->size;
        const IdxT nbytes = item->nbytes;
        LidxT const* indices = item->indices;
        for (DataT* dst : this->m_variables) {
          // LOGPRINTF("%p unpack %p[%p] = %p len %d\n", this, dst, indices, buf, len);
          this->m_contexts[msg->idx].for_all(len, make_copy_idxr_idxr(static_cast<DataT*>(static_cast<void*>(buf)), detail::indexer_i{},
                                             dst, detail::indexer_list_i{indices}));
          buf += nbytes;
        }
      }
      this->m_contexts[msg->idx].finish_component(this->m_groups[len-1], this->m_components[msg->idx]);
    }
    con.finish_group(this->m_groups[len-1]);
  }

  void deallocate(context_type& con, communicator_type& con_comm, message_type** msgs, IdxT len, detail::Async async)
  {
	  if(len <= 0) return;
	  
	  if(comb_allow_pack_loop_fusion())
	  {
		  this->m_fuser.deallocate(con);
	  }
  }
};


template < >
struct MessageGroup<MessageBase::Kind::send, mp_pol, mpi_type_pol>
{
  // unimplemented
};

template < >
struct MessageGroup<MessageBase::Kind::recv, mp_pol, mpi_type_pol>
{
  // unimplemented
};

} // namespace detail

#endif // COMB_ENABLE_MP

#endif // _COMM_POL_MP_HPP
