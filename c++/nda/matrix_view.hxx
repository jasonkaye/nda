//  File is generated by vim.
//   To regenerate the file, use in this buffer the vim script
//
//   :source c++/nda/matrix_view.vim
#pragma once
#include "./storage/handle.hpp"
#include "./indexmap/idx_map.hpp"
#include "./basic_functions.hpp"
#include "./assignment.hpp"
#include "./iterator_adapter.hpp"

namespace nda {

  // ---------------------- declare matrix and view  --------------------------------

  template <typename ValueType, uint64_t Layout = 0>
  class matrix;
  template <typename ValueType, uint64_t Guarantees = 0, uint64_t Layout = 0>
  class matrix_view;

  // ---------------------- is_matrix_or_view_container  --------------------------------

  template <typename ValueType>
  inline constexpr bool is_regular_v<matrix<ValueType>> = true;

  template <typename ValueType>
  inline constexpr bool is_regular_or_view_v<matrix<ValueType>> = true;

  template <typename ValueType, uint64_t Guarantees, uint64_t Layout>
  inline constexpr bool is_regular_or_view_v<matrix_view<ValueType, Guarantees, Layout>> = true;

  // ---------------------- concept  --------------------------------

  template <typename ValueType, uint64_t Layout>
  inline constexpr bool is_ndarray_v<matrix<ValueType, Layout>> = true;

  template <typename ValueType, uint64_t Guarantees, uint64_t Layout>
  inline constexpr bool is_ndarray_v<matrix_view<ValueType, Guarantees, Layout>> = true;

  // ---------------------- algebra --------------------------------

  template <typename ValueType, uint64_t Layout>
  inline constexpr char get_algebra<matrix<ValueType, Layout>> = 'A';

  template <typename ValueType, uint64_t Guarantees, uint64_t Layout>
  inline constexpr char get_algebra<matrix_view<ValueType, Guarantees, Layout>> = 'A';

  // ---------------------- guarantees --------------------------------

  template <typename ValueType>
  inline constexpr uint64_t get_guarantee<matrix<ValueType>> = matrix<ValueType>::guarantees;

  template <typename ValueType, uint64_t Guarantees, uint64_t Layout>
  inline constexpr uint64_t get_guarantee<matrix_view<ValueType, Guarantees, Layout>> = Guarantees;

  // ---------------------- matrix_view  --------------------------------

  // Try to put the const/mutable in the TYPE

  template <typename ValueType, uint64_t Guarantees, uint64_t Layout>
  class matrix_view {

    public:
    /// ValueType, without const if any
    using value_t = std::remove_const_t<ValueType>;
    ///
    using regular_t = matrix<value_t, Layout>;
    ///
    using view_t = matrix_view<value_t, Guarantees, Layout>;
    ///
    using const_view_t = matrix_view<value_t const, Guarantees, Layout>;

    //using value_as_template_arg_t = ValueType;
    using storage_t = mem::handle<value_t, 'B'>;
    using idx_map_t = idx_map<2, Layout>;

    static constexpr int rank      = 2;
    static constexpr bool is_view  = true;
    static constexpr bool is_const = std::is_const_v<ValueType>;

    static constexpr uint64_t guarantees = Guarantees; // for the generic shared with matrix
                                                       //    static constexpr uint64_t layout = Layout;

    // fIXME : FIRST STEP.
    static_assert(Guarantees == 0, "Not implemented");

    // FIXME : h5
    // static std::string hdf5_scheme() { return "matrix<" + triqs::h5::get_hdf5_scheme<value_t>() + "," + std::to_string(rank) + ">"; }

    private:
    template <typename IdxMap>
    using my_view_template_t = matrix_view<value_t, IdxMap::flags, permutations::encode(IdxMap::layout)>;

    idx_map_t _idx_m;
    storage_t _storage;

    public:
    // ------------------------------- constructors --------------------------------------------

    /// Construct an empty view.
    matrix_view() = default;

    ///
    matrix_view(matrix_view &&) = default;

    /// Shallow copy. It copies the *view*, not the data.
    matrix_view(matrix_view const &) = default;

    /** 
     * From a view of non const ValueType.
     * Only valid when ValueType is const
     *
     * @param v a view 
     */
    matrix_view(matrix_view<value_t> const &v) REQUIRES(is_const) : matrix_view(v.indexmap(), v.storage()) {}

    /**
     *  [Advanced] From an indexmap and a storage handle
     *  @param idxm index map
     *  @st  storage (memory handle)
     */
    matrix_view(idx_map<2, Layout> const &idxm, storage_t st) : _idx_m(idxm), _storage(std::move(st)) {}

    /** 
     * From other containers and view : matrix, matrix, matrix_view.
     *
     * @tparam A an matrix/matrix_view or matrix/vector type
     * @param a matrix or view
     */
    template <typename A> //explicit
    matrix_view(A const &a) REQUIRES(is_regular_or_view_v<A>) : matrix_view(a.indexmap(), a.storage()) {}

    /** 
     * [Advanced] From a pointer to contiguous data, and a shape.
     * NB : no control obvious on the dimensions given.  
     *
     * @param p Pointer to the data
     * @param shape Shape of the view (contiguous)
     */
    matrix_view(std::array<long, 2> const &shape, value_t *p) : matrix_view(idx_map_t{shape}, p) {}

    /** 
     * [Advanced] From a pointer to data, and an idx_map 
     * NB : no control obvious on the dimensions given.  
     *
     * @param p Pointer to the data 
     * @param idxm Index Map (view can be non contiguous). If the offset is non zero, the view starts at p + idxm.offset()
     */
    matrix_view(idx_map<2, Layout> const &idxm, ValueType *p) : _idx_m(idxm), _storage{p, size_t(idxm.size() + idxm.offset())} {}

    // Move assignment not defined : will use the copy = since view must copy data

    // ------------------------------- assign --------------------------------------------

    /**
     * Copies the content of rhs into the view.
     * Pseudo code : 
     *     for all i,j,k,l,... : this[i,j,k,l] = rhs(i,j,k,l)
     *
     * The dimension of RHS must be large enough or behaviour is undefined.
     * 
     * If NDA_BOUNDCHECK is defined, the bounds are checked.
     *
     * @tparam RHS A scalar or an object modeling the concept NDArray
     * @param rhs Right hand side of the = operation
     */
    template <typename RHS>
    matrix_view &operator=(RHS const &rhs) {
      nda::details::assignment(*this, rhs);
      return *this;
    }

    /// Same as the general case
    /// [C++ oddity : this case must be explicitly coded too]
    matrix_view &operator=(matrix_view const &rhs) {
      nda::details::assignment(*this, rhs);
      return *this;
    }

    // ------------------------------- rebind --------------------------------------------

    /// Rebind the view
    void rebind(matrix_view<value_t> const &a) { //value_t is NEVER const
      _idx_m   = a._idx_m;
      _storage = a._storage;
    }

    /// Rebind view
    void rebind(matrix_view<value_t const> const &a) {
      static_assert(is_const, "Can not rebind a view of const ValueType to a view of ValueType");
      _idx_m   = a._idx_m;
      _storage = a._storage;
    }
    //check https://godbolt.org/z/G_QRCU

    //----------------------------------------------------

#include "./_regular_view_common.hpp"
  };

  /*
  template <typename ValueType, uint64_t Flags, uint64_t Layout> class matrix_view : public matrix_view<ValueType, 0, Layout> {

    using B = matrix_view<ValueType, 0, Layout>;

    public:
   
    using B::B;
    using B::operator=;

    // redefine oeprator() 
 // add a cross constructor

  };
*/
  /// Aliases
  template <typename ValueType, uint64_t Guarantees = 0, uint64_t Layout = 0>
  using matrix_const_view = matrix_view<ValueType const, Layout, Guarantees>;

} // namespace nda
