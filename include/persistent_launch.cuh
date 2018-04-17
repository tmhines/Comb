
#ifndef _PERSISTENT_LAUNCH_CUH
#define _PERSISTENT_LAUNCH_CUH

#include "batch_utils.cuh"
#include "MultiBuffer.cuh"

namespace cuda {

namespace persistent_launch {

namespace detail {

inline ::detail::MultiBuffer& getMultiBuffer()
{
  static ::detail::MultiBuffer buf;
  return buf;
}

inline bool& getLaunched()
{
  static bool launched = false;
  return launched;
}

extern void launch(::detail::MultiBuffer& mb, cudaStream_t stream);
extern void stop(::detail::MultiBuffer& mb, cudaStream_t stream);

// Enqueue a kernel in the current buffer.
template <typename kernel_type_in>
inline void enqueue(::detail::MultiBuffer& mb, int begin, int n, kernel_type_in&& kernel_in, cudaStream_t stream = 0)
{
   using kernel_type = ::detail::kernel_holder_B_N<typename std::remove_reference<kernel_type_in>::type>;

   // write device wrapper function pointer to pinned buffer
   ::detail::device_wrapper_ptr wrapper_ptr = ::detail::get_device_wrapper_ptr<kernel_type>();
   
   // Copy kernel into kernel holder and write to pinned buffer
   kernel_type kernel{kernel_in, begin, n};
   
   if (!getLaunched()) {
     launch(mb, stream);
   }

   // Wait for next buffer to be writeable, then pack
   while ( !mb.pack(wrapper_ptr, &kernel) );
}

} // namespace detail

extern void force_start(cudaStream_t stream = 0);
extern void force_complete(cudaStream_t stream = 0);
extern void synchronize(cudaStream_t stream = 0);

template <typename kernel_type_in>
inline void for_all(int begin, int end, kernel_type_in&& kernel_in, cudaStream_t stream = 0 )
{
   if (begin < end) {
      detail::enqueue(detail::getMultiBuffer(), begin, end - begin, std::forward<kernel_type_in>(kernel_in), stream);
   }
}

} // namespace persistent_launch

} // namespace cuda

#endif // _PERSISTENT_LAUNCH_CUH
