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

#ifndef _MEMORY_HPP
#define _MEMORY_HPP

#include "config.hpp"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>
#include <stdexcept>

#include "basic_mempool.hpp"

#include "ExecContext.hpp"
#include "exec_utils_cuda.hpp"

namespace COMB {

namespace detail {

template < typename alloc >
using mempool = COMBRAJA::basic_mempool::MemPool<alloc>;

#ifdef COMB_ENABLE_CUDA
  struct cuda_host_pinned_allocator {
    void* malloc(size_t nbytes) {
      void* ptr = nullptr;
      cudaCheck(cudaHostAlloc(&ptr, nbytes, cudaHostAllocDefault));
      return ptr;
    }
    void free(void* ptr) {
      cudaCheck(cudaFreeHost(ptr));
    }
  };

  struct cuda_device_allocator {
    void* malloc(size_t nbytes) {
      void* ptr = nullptr;
      cudaCheck(cudaMalloc(&ptr, nbytes));
      return ptr;
    }
    void free(void* ptr) {
      cudaCheck(cudaFree(ptr));
    }
  };

  struct cuda_managed_allocator {
    void* malloc(size_t nbytes) {
      void* ptr = nullptr;
      cudaCheck(cudaMallocManaged(&ptr, nbytes));
      return ptr;
    }
    void free(void* ptr) {
      cudaCheck(cudaFree(ptr));
    }
  };

  struct cuda_managed_read_mostly_allocator {
    void* malloc(size_t nbytes) {
      void* ptr = nullptr;
      cudaCheck(cudaMallocManaged(&ptr, nbytes));
      cudaCheck(cudaMemAdvise(ptr, nbytes, cudaMemAdviseSetReadMostly, 0));
      return ptr;
    }
    void free(void* ptr) {
      cudaCheck(cudaFree(ptr));
    }
  };

  struct cuda_managed_host_preferred_allocator {
    void* malloc(size_t nbytes) {
      void* ptr = nullptr;
      cudaCheck(cudaMallocManaged(&ptr, nbytes));
      cudaCheck(cudaMemAdvise(ptr, nbytes, cudaMemAdviseSetPreferredLocation, cudaCpuDeviceId));
      return ptr;
    }
    void free(void* ptr) {
      cudaCheck(cudaFree(ptr));
    }
  };

  struct cuda_managed_host_preferred_device_accessed_allocator {
    void* malloc(size_t nbytes) {
      void* ptr = nullptr;
      cudaCheck(cudaMallocManaged(&ptr, nbytes));
      cudaCheck(cudaMemAdvise(ptr, nbytes, cudaMemAdviseSetPreferredLocation, cudaCpuDeviceId));
      cudaCheck(cudaMemAdvise(ptr, nbytes, cudaMemAdviseSetAccessedBy, cuda::get_device()));
      return ptr;
    }
    void free(void* ptr) {
      cudaCheck(cudaFree(ptr));
    }
  };

  struct cuda_managed_device_preferred_allocator {
    void* malloc(size_t nbytes) {
      void* ptr = nullptr;
      cudaCheck(cudaMallocManaged(&ptr, nbytes));
      cudaCheck(cudaMemAdvise(ptr, nbytes, cudaMemAdviseSetPreferredLocation, cuda::get_device()));
      return ptr;
    }
    void free(void* ptr) {
      cudaCheck(cudaFree(ptr));
    }
  };

  struct cuda_managed_device_preferred_host_accessed_allocator {
    void* malloc(size_t nbytes) {
      void* ptr = nullptr;
      cudaCheck(cudaMallocManaged(&ptr, nbytes));
      cudaCheck(cudaMemAdvise(ptr, nbytes, cudaMemAdviseSetPreferredLocation, cuda::get_device()));
      cudaCheck(cudaMemAdvise(ptr, nbytes, cudaMemAdviseSetAccessedBy, cudaCpuDeviceId));
      return ptr;
    }
    void free(void* ptr) {
      cudaCheck(cudaFree(ptr));
    }
  };
#endif

} // end detail

struct Allocator
{
  virtual const char* name() { return "Null"; }
  virtual void* allocate(size_t nbytes)
  {
    COMB::ignore_unused(nbytes);
    void* ptr = nullptr;
    // LOGPRINTF("allocated %p nbytes %zu\n", ptr, nbytes);
    return ptr;
  }
  virtual void deallocate(void* ptr)
  {
    // LOGPRINTF("deallocating %p\n", ptr);
    assert(ptr == nullptr);
  }
};

struct HostAllocator : Allocator
{
  const char* name() override { return "Host"; }
  void* allocate(size_t nbytes) override
  {
    void* ptr = malloc(nbytes);
    // LOGPRINTF("allocated %p nbytes %zu\n", ptr, nbytes);
    return ptr;
  }
  void deallocate(void* ptr) override
  {
    // LOGPRINTF("deallocating %p\n", ptr);
    free(ptr);
  }
};

struct HostPinnedAllocator : Allocator
{
#ifdef COMB_ENABLE_CUDA
  const char* name() override { return "HostPinned"; }
  void* allocate(size_t nbytes) override
  {
    void* ptr = detail::mempool<detail::cuda_host_pinned_allocator>::getInstance().malloc<char>(nbytes);
    return ptr;
  }
  void deallocate(void* ptr) override
  {
    detail::mempool<detail::cuda_host_pinned_allocator>::getInstance().free(ptr);
  }
#endif
};

struct DeviceAllocator : Allocator
{
#ifdef COMB_ENABLE_CUDA
  const char* name() override { return "Device"; }
  void* allocate(size_t nbytes) override
  {
    void* ptr = detail::mempool<detail::cuda_device_allocator>::getInstance().malloc<char>(nbytes);
    return ptr;
  }
  void deallocate(void* ptr) override
  {
    detail::mempool<detail::cuda_device_allocator>::getInstance().free(ptr);
  }
#endif
};

struct ManagedAllocator : Allocator
{
#ifdef COMB_ENABLE_CUDA
  const char* name() override { return "Managed"; }
  void* allocate(size_t nbytes) override
  {
    void* ptr = detail::mempool<detail::cuda_managed_allocator>::getInstance().malloc<char>(nbytes);
    return ptr;
  }
  void deallocate(void* ptr) override
  {
    detail::mempool<detail::cuda_managed_allocator>::getInstance().free(ptr);
  }
#endif
};

struct ManagedHostPreferredAllocator : Allocator
{
#ifdef COMB_ENABLE_CUDA
  const char* name() override { return "ManagedHostPreferred"; }
  void* allocate(size_t nbytes) override
  {
    void* ptr = detail::mempool<detail::cuda_managed_host_preferred_allocator>::getInstance().malloc<char>(nbytes);
    return ptr;
  }
  void deallocate(void* ptr) override
  {
    detail::mempool<detail::cuda_managed_host_preferred_allocator>::getInstance().free(ptr);
  }
#endif
};

struct ManagedHostPreferredDeviceAccessedAllocator : Allocator
{
#ifdef COMB_ENABLE_CUDA
  const char* name() override { return "ManagedHostPreferredDeviceAccessed"; }
  void* allocate(size_t nbytes) override
  {
    void* ptr = detail::mempool<detail::cuda_managed_host_preferred_device_accessed_allocator>::getInstance().malloc<char>(nbytes);
    return ptr;
  }
  void deallocate(void* ptr) override
  {
    detail::mempool<detail::cuda_managed_host_preferred_device_accessed_allocator>::getInstance().free(ptr);
  }
#endif
};

struct ManagedDevicePreferredAllocator : Allocator
{
#ifdef COMB_ENABLE_CUDA
  const char* name() override { return "ManagedDevicePreferred"; }
  void* allocate(size_t nbytes) override
  {
    void* ptr = detail::mempool<detail::cuda_managed_device_preferred_allocator>::getInstance().malloc<char>(nbytes);
    return ptr;
  }
  void deallocate(void* ptr) override
  {
    detail::mempool<detail::cuda_managed_device_preferred_allocator>::getInstance().free(ptr);
  }
#endif
};

struct ManagedDevicePreferredHostAccessedAllocator : Allocator
{
#ifdef COMB_ENABLE_CUDA
  const char* name() override { return "ManagedDevicePreferredHostAccessed"; }
  void* allocate(size_t nbytes) override
  {
    void* ptr = detail::mempool<detail::cuda_managed_device_preferred_host_accessed_allocator>::getInstance().malloc<char>(nbytes);
    return ptr;
  }
  void deallocate(void* ptr) override
  {
    detail::mempool<detail::cuda_managed_device_preferred_host_accessed_allocator>::getInstance().free(ptr);
  }
#endif
};


template < typename T >
struct std_allocator
{
  using value_type = T;

  std_allocator(Allocator* aloc) noexcept
    : m_aloc(aloc)
  { }

  template < typename U >
  constexpr std_allocator(std_allocator<U> const& other) noexcept
    : m_aloc(other.allocator())
  { }

  /*[[nodiscard]]*/
  value_type* allocate(size_t num)
  {
    if (num > std::numeric_limits<size_t>::max() / sizeof(value_type)) {
      throw std::bad_alloc();
    }

    value_type *ptr = static_cast<value_type*>(m_aloc->allocate(sizeof(value_type)*num));

    if (!ptr) {
      throw std::bad_alloc();
    }

    return ptr;
  }

  void deallocate(value_type* ptr, size_t) noexcept
  {
    m_aloc->deallocate(ptr);
  }

  Allocator* allocator() const noexcept { return m_aloc; }

private:
  Allocator* m_aloc = nullptr;
};

template <typename T, typename U>
bool operator==(std_allocator<T> const& lhs, std_allocator<U> const& rhs)
{
  return lhs.allocator() == rhs.allocator();
}

template <typename T, typename U>
bool operator!=(std_allocator<T> const& lhs, std_allocator<U> const& rhs)
{
  return !(lhs == rhs);
}


struct AllocatorAccessibilityFlags
{
  // special flag to enable tests that access host pageable memory from the device
  bool cuda_host_accessible_from_device = false;
  // special flag to enable tests that access device memory from the host
  bool cuda_device_accessible_from_host = false;
  // special flag to enable tests that pass device buffers to MPI
  bool cuda_aware_mpi = false;
  // special flag to allow use of device preferred host accessed managed memory for cuda util allocations
  bool use_device_preferred_for_cuda_util_aloc = false;
};

struct AllocatorInfo
{
  enum struct UseType : int
  {
    UseTypeBegin = 0,
    Mesh = 0,
    Buffer,
    UseTypeEnd
  };
  AllocatorInfo(AllocatorAccessibilityFlags& a) : m_accessFlags(a) { }
  void set_available(std::vector<COMB::AllocatorInfo::UseType> const& uts, bool avail)
  {
    for (COMB::AllocatorInfo::UseType ut : uts) {
      m_available[validate_and_convert(ut)] = avail;
    }
  }

  virtual Allocator& allocator() = 0;
  virtual bool available(UseType ut) = 0;
  virtual bool accessible(CPUContext const&) = 0;
#ifdef COMB_ENABLE_MPI
  virtual bool accessible(MPIContext const&) = 0;
#endif
#ifdef COMB_ENABLE_CUDA
  virtual bool accessible(CudaContext const&) = 0;
#endif
#ifdef COMB_ENABLE_RAJA
  virtual bool accessible(RAJAContext<RAJA::resources::Host> const&) = 0;
#ifdef COMB_ENABLE_CUDA
  virtual bool accessible(RAJAContext<RAJA::resources::Cuda> const&) = 0;
#endif
#endif

protected:
  bool m_available[static_cast<int>(UseType::UseTypeEnd)] = {false, false};
  AllocatorAccessibilityFlags& m_accessFlags;

  int validate_and_convert(UseType ut_in)
  {
    int ut = static_cast<int>(ut_in);
    assert(ut >= static_cast<int>(UseType::UseTypeBegin));
    assert(ut < static_cast<int>(UseType::UseTypeEnd));
    return ut;
  }
};

struct InvalidAllocatorInfo : AllocatorInfo
{
  InvalidAllocatorInfo(AllocatorAccessibilityFlags& a) : AllocatorInfo(a) { }
  Allocator& allocator() override { throw std::invalid_argument("InvalidAllocatorInfo has no allocator"); }
  bool available(UseType) override { return false; }
  bool accessible(CPUContext const&) override { return false; }
#ifdef COMB_ENABLE_MPI
  bool accessible(MPIContext const&) override { return false; }
#endif
#ifdef COMB_ENABLE_CUDA
  bool accessible(CudaContext const&) override { return false; }
#endif
#ifdef COMB_ENABLE_RAJA
  bool accessible(RAJAContext<RAJA::resources::Host> const&) override { return false; }
#ifdef COMB_ENABLE_CUDA
  bool accessible(RAJAContext<RAJA::resources::Cuda> const&) override { return false; }
#endif
#endif
};

struct HostAllocatorInfo : AllocatorInfo
{
  HostAllocatorInfo(AllocatorAccessibilityFlags& a) : AllocatorInfo(a) { }
  Allocator& allocator() override { return m_allocator; }
  bool available(UseType ut) override { return m_available[validate_and_convert(ut)]; }
  bool accessible(CPUContext const&) override { return true; }
#ifdef COMB_ENABLE_MPI
  bool accessible(MPIContext const&) override { return true; }
#endif
#ifdef COMB_ENABLE_CUDA
  bool accessible(CudaContext const&) override { return m_accessFlags.cuda_host_accessible_from_device; }
#endif
#ifdef COMB_ENABLE_RAJA
  bool accessible(RAJAContext<RAJA::resources::Host> const&) override { return true; }
#ifdef COMB_ENABLE_CUDA
  bool accessible(RAJAContext<RAJA::resources::Cuda> const&) override { return m_accessFlags.cuda_host_accessible_from_device; }
#endif
#endif
private:
  HostAllocator m_allocator;
};

#ifdef COMB_ENABLE_CUDA

struct HostPinnedAllocatorInfo : AllocatorInfo
{
  HostPinnedAllocatorInfo(AllocatorAccessibilityFlags& a) : AllocatorInfo(a) { }
  Allocator& allocator() override { return m_allocator; }
  bool available(UseType ut) override { return m_available[validate_and_convert(ut)]; }
  bool accessible(CPUContext const&) override { return true; }
#ifdef COMB_ENABLE_MPI
  bool accessible(MPIContext const&) override { return true; }
#endif
  bool accessible(CudaContext const&) override { return true; }
#ifdef COMB_ENABLE_RAJA
  bool accessible(RAJAContext<RAJA::resources::Host> const&) override { return true; }
  bool accessible(RAJAContext<RAJA::resources::Cuda> const&) override { return true; }
#endif
private:
  HostPinnedAllocator m_allocator;
};

struct DeviceAllocatorInfo : AllocatorInfo
{
  DeviceAllocatorInfo(AllocatorAccessibilityFlags& a) : AllocatorInfo(a) { }
  Allocator& allocator() override { return m_allocator; }
  bool available(UseType ut) override { return m_available[validate_and_convert(ut)]; }
  bool accessible(CPUContext const&) override { return m_accessFlags.cuda_device_accessible_from_host; }
#ifdef COMB_ENABLE_MPI
  bool accessible(MPIContext const&) override { return m_accessFlags.cuda_aware_mpi; }
#endif
  bool accessible(CudaContext const&) override { return true; }
#ifdef COMB_ENABLE_RAJA
  bool accessible(RAJAContext<RAJA::resources::Host> const&) override { return m_accessFlags.cuda_device_accessible_from_host; }
  bool accessible(RAJAContext<RAJA::resources::Cuda> const&) override { return true; }
#endif
private:
  DeviceAllocator m_allocator;
};

struct ManagedAllocatorInfo : AllocatorInfo
{
  ManagedAllocatorInfo(AllocatorAccessibilityFlags& a) : AllocatorInfo(a) { }
  Allocator& allocator() override { return m_allocator; }
  bool available(UseType ut) override { return m_available[validate_and_convert(ut)]; }
  bool accessible(CPUContext const&) override { return true; }
#ifdef COMB_ENABLE_MPI
  bool accessible(MPIContext const&) override { return m_accessFlags.cuda_aware_mpi; }
#endif
  bool accessible(CudaContext const&) override { return true; }
#ifdef COMB_ENABLE_RAJA
  bool accessible(RAJAContext<RAJA::resources::Host> const&) override { return true; }
  bool accessible(RAJAContext<RAJA::resources::Cuda> const&) override { return true; }
#endif
private:
  ManagedAllocator m_allocator;
};

struct ManagedHostPreferredAllocatorInfo : AllocatorInfo
{
  ManagedHostPreferredAllocatorInfo(AllocatorAccessibilityFlags& a) : AllocatorInfo(a) { }
  Allocator& allocator() override { return m_allocator; }
  bool available(UseType ut) override { return m_available[validate_and_convert(ut)] && detail::cuda::get_concurrent_managed_access(); }
  bool accessible(CPUContext const&) override { return true; }
#ifdef COMB_ENABLE_MPI
  bool accessible(MPIContext const&) override { return m_accessFlags.cuda_aware_mpi; }
#endif
  bool accessible(CudaContext const&) override { return true; }
#ifdef COMB_ENABLE_RAJA
  bool accessible(RAJAContext<RAJA::resources::Host> const&) override { return true; }
  bool accessible(RAJAContext<RAJA::resources::Cuda> const&) override { return true; }
#endif
private:
  ManagedHostPreferredAllocator m_allocator;
};

struct ManagedHostPreferredDeviceAccessedAllocatorInfo : AllocatorInfo
{
  ManagedHostPreferredDeviceAccessedAllocatorInfo(AllocatorAccessibilityFlags& a) : AllocatorInfo(a) { }
  Allocator& allocator() override { return m_allocator; }
  bool available(UseType ut) override { return m_available[validate_and_convert(ut)] && detail::cuda::get_concurrent_managed_access(); }
  bool accessible(CPUContext const&) override { return true; }
#ifdef COMB_ENABLE_MPI
  bool accessible(MPIContext const&) override { return m_accessFlags.cuda_aware_mpi; }
#endif
  bool accessible(CudaContext const&) override { return true; }
#ifdef COMB_ENABLE_RAJA
  bool accessible(RAJAContext<RAJA::resources::Host> const&) override { return true; }
  bool accessible(RAJAContext<RAJA::resources::Cuda> const&) override { return true; }
#endif
private:
  ManagedHostPreferredDeviceAccessedAllocator m_allocator;
};

struct ManagedDevicePreferredAllocatorInfo : AllocatorInfo
{
  ManagedDevicePreferredAllocatorInfo(AllocatorAccessibilityFlags& a) : AllocatorInfo(a) { }
  Allocator& allocator() override { return m_allocator; }
  bool available(UseType ut) override { return m_available[validate_and_convert(ut)] && detail::cuda::get_concurrent_managed_access(); }
  bool accessible(CPUContext const&) override { return true; }
#ifdef COMB_ENABLE_MPI
  bool accessible(MPIContext const&) override { return m_accessFlags.cuda_aware_mpi; }
#endif
  bool accessible(CudaContext const&) override { return true; }
#ifdef COMB_ENABLE_RAJA
  bool accessible(RAJAContext<RAJA::resources::Host> const&) override { return true; }
  bool accessible(RAJAContext<RAJA::resources::Cuda> const&) override { return true; }
#endif
private:
  ManagedDevicePreferredAllocator m_allocator;
};

struct ManagedDevicePreferredHostAccessedAllocatorInfo : AllocatorInfo
{
  ManagedDevicePreferredHostAccessedAllocatorInfo(AllocatorAccessibilityFlags& a) : AllocatorInfo(a) { }
  Allocator& allocator() override { return m_allocator; }
  bool available(UseType ut) override { return m_available[validate_and_convert(ut)] && detail::cuda::get_concurrent_managed_access(); }
  bool accessible(CPUContext const&) override { return true; }
#ifdef COMB_ENABLE_MPI
  bool accessible(MPIContext const&) override { return m_accessFlags.cuda_aware_mpi; }
#endif
  bool accessible(CudaContext const&) override { return true; }
#ifdef COMB_ENABLE_RAJA
  bool accessible(RAJAContext<RAJA::resources::Host> const&) override { return true; }
  bool accessible(RAJAContext<RAJA::resources::Cuda> const&) override { return true; }
#endif
private:
  ManagedDevicePreferredHostAccessedAllocator m_allocator;
};

#endif

struct Allocators
{
  AllocatorAccessibilityFlags access;

  InvalidAllocatorInfo                            invalid{access};
  HostAllocatorInfo                               host{access};
#ifdef COMB_ENABLE_CUDA
  HostPinnedAllocatorInfo                         cuda_hostpinned{access};
  DeviceAllocatorInfo                             cuda_device{access};
  ManagedAllocatorInfo                            cuda_managed{access};
  ManagedHostPreferredAllocatorInfo               cuda_managed_host_preferred{access};
  ManagedHostPreferredDeviceAccessedAllocatorInfo cuda_managed_host_preferred_device_accessed{access};
  ManagedDevicePreferredAllocatorInfo             cuda_managed_device_preferred{access};
  ManagedDevicePreferredHostAccessedAllocatorInfo cuda_managed_device_preferred_host_accessed{access};
#endif
};

} // namespace COMB

#endif // _MEMORY_HPP

