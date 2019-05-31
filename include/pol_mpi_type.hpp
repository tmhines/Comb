//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018, Lawrence Livermore National Security, LLC.
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

#ifndef _POL_MPI_TYPE_HPP
#define _POL_MPI_TYPE_HPP

// execution policy indicating that message packing/unpacking should be done
// in MPI using MPI_Types
struct mpi_type_pol {
  static const bool async = false;
  static const char* get_name() { return "mpi_type"; }
  using event_type = int;
};

// synchronization functions
inline void synchronize(mpi_type_pol const&)
{
}

// force start functions
inline void persistent_launch(mpi_type_pol const&)
{
}

// force complete functions
inline void batch_launch(mpi_type_pol const&)
{
}

// force complete functions
inline void persistent_stop(mpi_type_pol const&)
{
}

// event creation functions
inline typename mpi_type_pol::event_type createEvent(mpi_type_pol const&)
{
  return typename mpi_type_pol::event_type{};
}

// event record functions
inline void recordEvent(mpi_type_pol const&, typename mpi_type_pol::event_type)
{
}

// event query functions
inline bool queryEvent(mpi_type_pol const&, typename mpi_type_pol::event_type)
{
  return true;
}

// event wait functions
inline void waitEvent(mpi_type_pol const&, typename mpi_type_pol::event_type)
{
}

// event destroy functions
inline void destroyEvent(mpi_type_pol const&, typename mpi_type_pol::event_type)
{
}

// template < typename body_type >
// inline void for_all(mpi_type_pol const& pol, IdxT begin, IdxT end, body_type&& body)
// {
//   COMB::ignore_unused(pol, begin, end, body);
//   static_assert(false, "This method should never be used");
// }

// template < typename body_type >
// void for_all_2d(mpi_type_pol const& pol, IdxT begin0, IdxT end0, IdxT begin1, IdxT end1, body_type&& body)
// {
//   COMB::ignore_unused(pol, begin0, end0, begin1, end1, body);
//   static_assert(false, "This method should never be used");
// }

// template < typename body_type >
// void for_all_3d(mpi_type_pol const& pol, IdxT begin0, IdxT end0, IdxT begin1, IdxT end1, IdxT begin2, IdxT end2, body_type&& body)
// {
//   COMB::ignore_unused(pol, begin0, end0, begin1, end1, begin2, end2, body);
//   static_assert(false, "This method should never be used");
// }

#endif // _POL_MPI_TYPE_HPP