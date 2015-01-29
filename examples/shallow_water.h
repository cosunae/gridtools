#pragma once

#include <gridtools.h>
#include <common/halo_descriptor.h>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/fusion/include/make_vector.hpp>

#ifdef CUDA_EXAMPLE
#include <stencil-composition/backend_cuda.h>
#else
#include <stencil-composition/backend_host.h>
#endif

#ifdef CUDA_EXAMPLE
#include <boundary-conditions/apply_gpu.h>
#else
#include <boundary-conditions/apply.h>
#endif

/*
  @file
  @brief This file shows an implementation of the "shallow water" stencil, with periodic boundary conditions
  It defines
 */

using gridtools::level;
using gridtools::arg_type;
using gridtools::range;
using gridtools::arg;

using gridtools::direction;
using gridtools::sign;
using gridtools::minus_;
using gridtools::zero_;
using gridtools::plus_;

using namespace gridtools;
using namespace enumtype;
using namespace expressions;

namespace shallow_water{
// This is the definition of the special regions in the "vertical" direction
    typedef gridtools::interval<level<0,-1>, level<1,-1> > x_interval;
    typedef gridtools::interval<level<0,-2>, level<1,1> > axis;

/**@brief This traits class defined the necessary typesand functions used by all the functors defining the shallow water model*/
    struct functor_traits{
//#if  !((defined(__GNUC__)) && (__GNUC__ < 4) || (__GNUC__ == 4 && __GNUC_MINOR__ < 9))
        //using step=Dimension<3> ;
        using comp=Dimension<4>;
//#endif

	/**@brief space discretization step in direction i */
	GT_FUNCTION
        static float_type dx(){return 1.;}
	/**@brief space discretization step in direction j */
	GT_FUNCTION
        static float_type dy(){return 1.;}
	/**@brief time discretization step */
	GT_FUNCTION
        static float_type dt(){return .02;}
	/**@brief gravity acceleration */
	GT_FUNCTION
        static float_type g(){return 9.81;}



        template<typename Sol, typename Evaluation, typename ComponentU, typename DimensionX, typename DimensionY>
        GT_FUNCTION
        static float_type /*&&*/ half_step(Evaluation const& eval, ComponentU&& U, DimensionX&& d1, DimensionY&& d2, float_type const& delta)
            {
                return /*std::move*/(eval(Sol(d1,d2) +Sol(d2)/2. -
                                          (Sol(U,d2,d1) - Sol(U,d2))*(dt()/(2*delta))));
            }

        template<typename Sol, typename Evaluation, typename ComponentU, typename DimensionX, typename DimensionY>
        GT_FUNCTION
        static float_type /*&&*/ half_step_u(Evaluation const& eval, ComponentU&& U, DimensionX&& d1, DimensionY&& d2, float_type const& delta)
            {
                return /*std::move*/(eval((Sol(U, d1, d2) +
					   Sol(U, d2)/2. -
					   (pow<2>(Sol(U,d1,d2))/Sol(d1,d2)+pow<2>(Sol(d1,d2))*g()/2. -
					    pow<2>(Sol(U, d2))/Sol(d2) +
					    pow<2>(Sol(d2))*(g()/2.)))*(dt()/(2.*delta))) );
            }

        template<typename Sol, typename Evaluation, typename ComponentU, typename ComponentV, typename DimensionX, typename DimensionY>
        GT_FUNCTION
        static float_type/*&&*/ half_step_v(Evaluation const& eval, ComponentU&& U, ComponentV&& V, DimensionX&& d1, DimensionY&& d2, float_type const& delta)
            {
                return /*std::move*/(eval( Sol(V,d1,d2) +
					   Sol(V,d1)/2. -
					   (Sol(U,d1,d2)*Sol(V,d1,d2)/Sol(d1,d2) -
					    Sol(U,d2)*Sol(V,d2)/Sol(d2))*(dt()/(2*delta)) ) );
            }

    };

    template<uint_t Component=0, uint_t Snapshot=0>
    struct bc_periodic : functor_traits {
        // periodic boundary conditions in I
        template <sign I, sign K, typename DataField0>
        GT_FUNCTION
        void operator()(direction<I, minus_, K, typename boost::enable_if_c<I!=minus_>::type>,
                        DataField0 & data_field0,
                        uint_t i, uint_t j, uint_t k) const {
	    // TODO use placeholders here instead of the storage
	    data_field0.template get<Component, Snapshot>()[data_field0._index(i,j,k)] = data_field0.template get<Component, Snapshot>()[data_field0._index(i,data_field0.template dims<1>()-1-j,k)];
        }

        // periodic boundary conditions in J
        template <sign J, sign K, typename DataField0>
        GT_FUNCTION
        void operator()(direction<minus_, J, K>,
                        DataField0 & data_field0,
                        uint_t i, uint_t j, uint_t k) const {
	    // TODO use placeholders here instead of the storage
	    data_field0.template get<Component, Snapshot>()[data_field0._index(i,j,k)] = data_field0.template get<Component, Snapshot>()[data_field0._index(data_field0.template dims<0>()-1-i,j,k)];
        }

	// default: do nothing
        template <sign I, sign J, sign K, typename P, typename DataField0>
        GT_FUNCTION
        void operator()(direction<I, J, K, P>,
                        DataField0 & data_field0,
                        uint_t i, uint_t j, uint_t k) const {
        }

	static constexpr float_type height=2.;
	GT_FUNCTION
    	static float_type droplet(uint_t const& i, uint_t const& j, uint_t const& k){
            if(i>0 && j>0 && i<4 && j<4)
                return 1.+height * std::exp(-5*(((i-1)*dx())*(((i-1)*dx()))+((j-1)*dy())*((j-1)*dy())));
            else
                return 1.;
       }

};

    // struct bc : functor_traits{
    //     // periodic boundary conditions in K
    //     typedef arg_type<0> bc1;
    //     typedef arg_type<0> bc2;
    // 	static const float_type height=3.;

    // 	GT_FUNCTION
    // 	static float_type droplet(uint_t const& i, uint_t const& j, uint_t const& k){
    // 	    return height * std::exp(-5*((i*dx())*((i*dx())+(j*dy())*(j*dy())));
    // 	}

    //     template < typename Evaluation>
    //     GT_FUNCTION
    // 	static void Do(Evaluation const & eval, x_interval) {
    // 	    {
    // 		bc1() = bc2;
    // 	    }
    // };

    // struct bc_vertical{
    //     // periodic boundary conditions in K
    //     template < sign K, typename DataField0, typename DataField1>
    //     GT_FUNCTION
    //     void operator()(direction<zero_, zero_, K>,
    //                     DataField0 & data_field0, DataField1 const & data_field1,
    //                     uint_t i, uint_t j, uint_t k) const {
    //         data_field0(i,j,k) = data_field1(i,j,k);
    //     }
    // };

// These are the stencil operators that compose the multistage stencil in this test
    struct first_step_x        : public functor_traits {
        /**GCC 4.8.2  bug: inheriting the 'using' aliases (or replacing the typedefs below with the 'using' syntax) from the base class produces an internal compiler error (segfault).
           The compilation runs fine without warnings with GCC >= 4.9 and Clang*/

        typedef arg_extend<arg_type<0>, 2>::type tmpx;
        typedef arg_extend<arg_type<1>, 2>::type sol;
        using arg_list=boost::mpl::vector<tmpx, sol> ;

#if  (defined(__GNUC__)) && (__GNUC__ < 4) || (__GNUC__ == 4 && __GNUC_MINOR__ < 9)
        //shielding the base class aliases
        //typedef Dimension<3> step;
        typedef Dimension<4> comp;
#endif
        /* static const auto expression=in(1,0,0)-out(); */

        template <typename Evaluation>
        GT_FUNCTION
        static void Do(Evaluation const & eval, x_interval) {
        auto hx=alias<tmpx, comp>(0); auto h=alias<sol, comp>(0);
        auto ux=alias<tmpx, comp>(1); auto u=alias<sol, comp>(1);
        auto vx=alias<tmpx, comp>(2); auto v=alias<sol, comp>(2);

        x::Index i;
        y::Index j;

        eval(hx())=eval((h(i+1,j+1) +h(j+1))/2. -
            (u(i+1,j+1) - u(j+1))*(dt()/(2*dx())));
        // //eval(tmpx()       )=half_step<sol>  (eval, comp(1), x(1), y(1), dx());
        eval(ux())=eval(u(i+1, j+1) +
                        u(j+1)/2.-
                        ((pow<2>(u(i+1,j+1))/h(i+1,j+1)+pow<2>(h(i+1,j+1))*g()/2.)  -
                         (pow<2>(u(j+1))/h(j+1) +
                         pow<2>(h(j+1))*(g()/2.)
                             ))*(dt()/(2.*dx())));

        eval(vx())=eval( (v(i+1,j+1) +
                          v(j+1))/2. -
                         (u(i+1,j+1)*v(i+1,j+1)/h(i+1,j+1) -
                          u(j+1)*v(j+1)/h(j+1))*(dt()/(2*dx())) );
            //half_step_u<sol>(eval, comp(1), x(1), y(1), dx());
        // eval(tmpx(comp(2)))=half_step_v<sol>(eval, comp(1), comp(2), x(1), y(1), dx());
        }

    // 	void to_string(){
    // 	    (sol(V,d1,d2) +
    // 	     sol(V,d1)/2. -
    // 	     (sol(U,d1,d2)*sol(V,d1,d2)/sol(d1,d2) -
    // 	      sol(U,d2)*sol(V,d2)/sol(d2))*(dt()/(2*delta)) )).to_string();
    // }
    };


    struct second_step_y        : public functor_traits {

        typedef arg_extend<arg_type<0>, 2>::type tmpy;
        typedef arg_extend<arg_type<1>, 2>::type sol;
        using arg_list=boost::mpl::vector<tmpy, sol> ;

        template <typename Evaluation>
        GT_FUNCTION
        static void Do(Evaluation const & eval, x_interval) {

        auto hy=alias<tmpy, comp>(0); auto h=alias<sol, comp>(0);
        auto uy=alias<tmpy, comp>(1); auto u=alias<sol, comp>(1);
        auto vy=alias<tmpy, comp>(2); auto v=alias<sol, comp>(2);

        x::Index i;
        y::Index j;

        eval(hy())= eval((h(i+1,j+1) + h(i+1))/2. -
                         (v(i+1,j+1) - v(i+1))*(dt()/(2*dy())) );

        eval(uy())=eval( (u(i+1,j+1) +
                          u(i+1))/2. -
                         (v(i+1,j+1)*u(i+1,j+1)/h(i+1,j+1) -
                          v(i+1)*u(i+1)/h(i+1))*(dt()/(2*dy())) );

        eval(vy())=eval(v(i+1, j+1) +
                        v(i+1)/2.-
                        ((pow<2>(v(i+1,j+1))/h(i+1,j+1)+pow<2>(h(i+1,j+1))*g()/2.)  -
                         (pow<2>(v(i+1))/h(i+1) +
                          pow<2>(h(i+1))*(g()/2.)
                             ))*(dt()/(2.*dy())));
        // eval(hy())=eval(h(i+1,j+1) +h(i+1)/2. -
        //                           (v(i+1,j+1) - v(i+1))*(dt()/(2*dy())));

        // //eval(tmpy(comp(0)))=half_step<sol>  (eval, comp(2), y(1), x(1), dy());
        // eval(tmpy(comp(1)))=half_step_v<sol>(eval, comp(2), comp(1), y(1), x(1), dy());
        // eval(tmpy(comp(2)))=half_step_u<sol>(eval, comp(2), y(1), x(1), dy());
        }
    };

    struct final_step        : public functor_traits {

        typedef arg_extend<arg_type<0>, 2>::type tmpx;
        typedef arg_extend<arg_type<1>, 2>::type tmpy;
        typedef arg_extend<arg_type<2>, 2>::type sol;
        // typedef arg_extend<arg_type<0, range<-1, 1, -1, 1> >, 2>::type tmp;
        // typedef arg_extend<arg_type<1, range<-1, 1, -1, 1> >, 2>::type sol;
        typedef boost::mpl::vector<tmpx, tmpy, sol> arg_list;

#if  (defined(__GNUC__)) && (__GNUC__ < 4) || (__GNUC__ == 4 && __GNUC_MINOR__ < 9)
        //typedef Dimension<3> step;
        typedef Dimension<4> comp;
#endif
	static uint_t current_time;

        template <typename Evaluation>
        GT_FUNCTION
        static void Do(Evaluation const & eval, x_interval) {
            //########## FINAL STEP #############
            //data dependencies with the previous parts
            //notation: alias<tmp, comp, step>(0, 0) is ==> tmp(comp(0), step(0)).
            //Using a strategy to define some arguments beforehand

            x::Index i;
            y::Index j;
#ifdef __CUDACC__
            comp::Index c;
            //step::Index s;

            eval(sol()) = eval(sol()-
                               (tmpx(c+1, i-1) - tmpx(c+1, i-1, j-1))*(dt()/dx())-
	    		       (tmpy(c+2, i/**/-1) - tmpy(c+2, i-1, j-1))*(dt()/dy())/**/);

            eval(sol(comp(1))) = eval(sol(c+1)   -
				      (pow<2>(tmpx(c+1, j-1))                / tmpx(j-1)     + tmpx(j-1)*tmpx(j-1)*((g()/2.))                 -
				       (pow<2>(tmpx(c+1,i-1,j-1))            / tmpx(i-1, j-1) +pow<2>(tmpx(i-1,j-1) )*((g()/2.))))*((dt()/dx())) -
				      (tmpy(c+2,i-1)*tmpy(c+1,i-1)          / tmpy(i-1)                                                   -
				       tmpy(c+2,i-1, j-1)*tmpy(c+1,i-1,j-1) / tmpy(i-1, j-1))*((dt()/dy())));/**/
// + tmp(s+1,i-1, j-1)*((g()/2.)))    *((dt()/dy())));

            eval(sol(comp(2))) = eval(sol(comp(2)) -
	    			      (tmpx(c+1,j-1)    *tmpx(c+2,j-1)       /tmpy(j-1) -
                                       (tmpx(c+1,i-1,j-1)*tmpx(c+2,i-1, j-1)) /tmpx(i-1, j-1))*((dt()/dx()))-
                                      (pow<2>(tmpy(c+2,i-1))                /tmpy(i-1)      +pow<2>(tmpy(i-1)     )*((g()/2.)) -
                                       pow<2>(tmpy(c+2,i-1,j-1))           /tmpy(i-1,j-1) +pow<2>(tmpy(i-1, j-1))*((g()/2.))   )*((dt()/dy())));
#else

	    auto hx=alias<tmpx, comp>(0); auto hy=alias<tmpy, comp>(0);
            auto ux=alias<tmpx, comp>(1); auto uy=alias<tmpy, comp>(1);
            auto vx=alias<tmpx, comp>(2); auto vy=alias<tmpy, comp>(2);

            eval(sol()) = eval(sol()-
                               (ux(j-1) - ux(i-1, j-1))*(dt()/dx())
                               -
                               (vy(i-1) - vy(i-1, j-1))*(dt()/dy())
                );

            eval(sol(comp(1))) = eval(sol(comp(1)) -
                                       (pow<2>(ux(j-1))                / hx(j-1)      + hx(j-1)*hx(j-1)*((g()/2.))                 -
	    			       (pow<2>(ux(i-1,j-1))            / hx(i-1, j-1) +pow<2>(hx(i-1,j-1) )*((g()/2.))))*((dt()/dx())) -
                                              (vy(i-1)*uy(i-1)          / hy(i-1)                                                   -
                                               vy(i-1, j-1)*uy(i-1,j-1) / hy(i-1, j-1)) *(dt()/dy()));

            eval(sol(comp(2))) = eval(sol(comp(2)) -
                                       (ux(j-1)    *vx(j-1)       /hx(j-1) -
                                        (ux(i-1,j-1)*vx(i-1, j-1)) /hx(i-1, j-1))*((dt()/dx()))-
                                      (pow<2>(vy(i-1))                /hy(i-1)      +pow<2>(hy(i-1)     )*((g()/2.)) -
                                       (pow<2>(vy(i-1, j-1))           /hy(i-1, j-1) +pow<2>(hy(i-1, j-1))*((g()/2.))   ))*((dt()/dy())));
#endif

    	}

    };

    uint_t final_step::current_time=0;

/*
 * The following operators and structs are for debugging only
 */
    std::ostream& operator<<(std::ostream& s, first_step_x const) {
        return s << "initial step 1: ";
	// initiali_step.to_string();
    }

    std::ostream& operator<<(std::ostream& s, second_step_y const) {
        return s << "initial step 2: ";
    }

/*
 * The following operators and structs are for debugging only
 */
    std::ostream& operator<<(std::ostream& s, final_step const) {
        return s << "final step";
    }

    extern char const s1[]="hello ";
    extern char const s2[]="world ";

    bool test(uint_t x, uint_t y, uint_t z) {
        {
#ifndef __CUDACC__
	    //testing the static printing
	    typedef string_c<print, s1, s2, s1, s1 > s;
	    s::apply();
#endif

            uint_t d1 = x;
            uint_t d2 = y;
            uint_t d3 = z;

#ifdef CUDA_EXAMPLE
#define BACKEND backend<Cuda, Naive >
#else
#ifdef BACKEND_BLOCK
#define BACKEND backend<Host, Block >
#else
#define BACKEND backend<Host, Naive >
#endif
#endif
            //                      dims  z y x
            //                   strides xy x 1
            typedef gridtools::layout_map<2,1,0> layout_t;
            typedef gridtools::BACKEND::storage_type<float_type, layout_t >::type storage_type;

    /* The nice interface does not compile today (CUDA 6.5) with nvcc (C++11 support not complete yet)*/
#ifdef __CUDACC__
//pointless and tedious syntax, temporary while thinking/waiting for an alternative like below
	    typedef base_storage<Cuda, float_type, layout_t, false ,6> base_type1;
	    typedef extend_width<base_type1, 1>  extended_type;
	    typedef extend_dim<extended_type, extended_type, extended_type>  tmp_type;

	    typedef base_storage<Cuda, float_type, layout_t, false ,3> base_type2;
	    typedef extend_width<base_type2, 0>  extended_type2;
	    typedef extend_dim<extended_type2, extended_type2, extended_type2>  sol_type;
#else
	    //typedef field<storage_type::basic_type, 1, 1, 1>::type tmp_type;
            typedef field<storage_type::basic_type, 1, 1, 1>::type sol_type;
#endif
	    typedef sol_type::original_storage::pointer_type ptr;

            // Definition of placeholders. The order of them reflects the order the user will deal with them
            // especially the non-temporary ones, in the construction of the domain
            // typedef arg<0, tmp_type > p_tmp;
            typedef arg<0, sol_type > p_tmpx;
            typedef arg<1, sol_type > p_tmpy;
            typedef arg<2, sol_type > p_sol;
            typedef boost::mpl::vector<p_tmpx, p_tmpy, p_sol> arg_type_list;


            // // Definition of the actual data fields that are used for input/output
            sol_type tmpx(d1-1,d2-1,d3);
            sol_type tmpy(d1-1,d2-1,d3);
            ptr out1(tmpx.size()), out2(tmpx.size()), out3(tmpx.size()), out4(tmpy.size()), out5(tmpy.size()), out6(tmpy.size());

            sol_type sol(d1,d2,d3);
            ptr out7(sol.size()), out8(sol.size()), out9(sol.size());

	    tmpx.set<0>(out1, 0.);
	    tmpx.set<1>(out2, 0.);
	    tmpx.set<2>(out3, 0.);

	    tmpy.set<0>(out4, 0.);
	    tmpy.set<1>(out5, 0.);
	    tmpy.set<2>(out6, 0.);

            sol.set<0>(out7, &bc_periodic<0,0>::droplet);//h
            sol.set<1>(out8, 0.);//u
            sol.set<2>(out9, 0.);//v

            std::cout<<"INITIALIZED VALUES"<<std::endl;
            sol.print();
            std::cout<<"#####################################################"<<std::endl;

            // sol.push_front<3>(out9, [](uint_t i, uint_t j, uint_t k) ->float_type {return 2.0*exp (-5*(i^2+j^2));});//h

            // construction of the domain. The domain is the physical domain of the problem, with all the physical fields that are used, temporary and not
            // It must be noted that the only fields to be passed to the constructor are the non-temporary.
            // The order in which they have to be passed is the order in which they appear scanning the placeholders in order. (I don't particularly like this)
            gridtools::domain_type<arg_type_list> domain
                (boost::fusion::make_vector(&tmpx, &tmpy, &sol));

// ///////////////////BOUNDARY CONDITIONS (TEST)///////////////////////
//             uint_t d0[5] = {0, 0, 0, 0, 0};
//             uint_t d_span_x[5] = {0, 0, 0, d1-1, d1};
//             uint_t d_span_y[5] = {0, 0, 0, d2-1, d2};

//             typedef gridtools::layout_map<-1,-1,0> layout_x;//2D storage
// 	    gridtools::coordinates<axis> coords_bc_x(d_span_x, d0);
// 	    coords.value_list[0] = 0;//only on k=0 (top)
// 	    coords.value_list[1] = 0;

// 	    typedef gridtools::layout_map<-1,0,-1> layout_y;//2D storage
// 	    gridtools::coordinates<axis> coords_bc_y(d0, d_span_y);
// 	    coords.value_list[0] = 0;//only on k=0 (top)
// 	    coords.value_list[1] = 0;

// 	    typedef gridtools::BACKEND::storage_type<float_type, layout_x >::type storage_bc_x;
// 	    typedef gridtools::BACKEND::storage_type<float_type, layout_y >::type storage_bc_y;

// 	    typedef arg<0, storage_bc_x > p_bc_x;
// 	    typedef arg<0, storage_bc_y > p_bc_y;
//             typedef boost::mpl::vector<p_bc_x> arg_type_list;
//             gridtools::domain_type<arg_type_list> domain
//                 (boost::fusion::make_vector(&tmp, &sol));

// 	    auto bc_x =
//                 gridtools::make_computation<gridtools::BACKEND, layout_x>
//                 (
//                     gridtools::make_mss // mss_descriptor
//                     (
//                         execute<forward>(),
//                         gridtools::make_esf<bc>(p_sol(), p_sol()) ,
//                         ),
//                     domain, coords_bc_x
//                     );

// 	    auto bc_y =
//                 gridtools::make_computation<gridtools::BACKEND, layout_y>
//                 (
//                     gridtools::make_mss // mss_descriptor
//                     (
//                         execute<forward>(),
//                         gridtools::make_esf<bc>(p_sol(), p_sol()) ,
//                         ),
//                     domain, coords_bc_y
//                     );
// ///////////////////END BOUNDARY CONDITIONS///////////////////////

            // Definition of the physical dimensions of the problem.
            // The constructor takes the horizontal plane dimensions,
            // while the vertical ones are set according the the axis property soon after
            // gridtools::coordinates<axis> coords(2,d1-2,2,d2-2);
            uint_t di[5] = {0, 0, 0, d1-1, d1};
            uint_t dj[5] = {0, 0, 0, d2-2, d2};
            gridtools::coordinates<axis> coords1(di, dj);
            coords1.value_list[0] = 0;
            coords1.value_list[1] = d3-1;

            auto shallow_water_stencil1 =
                gridtools::make_computation<gridtools::BACKEND, layout_t>
                (
                    gridtools::make_mss // mss_descriptor
                    (
                        execute<forward>(),
                        gridtools::make_esf<first_step_x> (p_tmpx(), p_sol() )
                        ),
                    domain, coords1
                    );

            uint_t di1[5] = {0, 0, 0, d1-2, d1};
            uint_t dj1[5] = {0, 0, 0, d2-1, d2};
            gridtools::coordinates<axis> coords2(di1, dj1);
            coords2.value_list[0] = 0;
            coords2.value_list[1] = d3-1;

            auto shallow_water_stencil2 =
                gridtools::make_computation<gridtools::BACKEND, layout_t>
                (
                    gridtools::make_mss // mss_descriptor
                    (
                        execute<forward>(),
                        gridtools::make_esf<second_step_y>(p_tmpy(), p_sol() )
                        ),
                    domain, coords2
                    );

            uint_t di2[5] = {1, 0, 1, d1-1, d1};
            uint_t dj2[5] = {1, 0, 1, d2-1, d2};
            gridtools::coordinates<axis> coords(di2, dj2);
            coords.value_list[0] = 0;
            coords.value_list[1] = d3-1;

            auto shallow_water_stencil =
                gridtools::make_computation<gridtools::BACKEND, layout_t>
                (
                    gridtools::make_mss // mss_descriptor
                    (
                        execute<forward>(),
                        gridtools::make_esf<final_step>(p_tmpx(), p_tmpy(), p_sol() )
                        ),
                    domain, coords
                    );

            shallow_water_stencil1->ready();
            shallow_water_stencil2->ready();
            shallow_water_stencil->ready();

            shallow_water_stencil1->steady();
            shallow_water_stencil2->steady();
            shallow_water_stencil->steady();

            gridtools::array<gridtools::halo_descriptor, 3> halos;
            halos[0] = gridtools::halo_descriptor(1,0,1,d1-1,d1);
            halos[1] = gridtools::halo_descriptor(1,0,1,d2-1,d2);
            halos[2] = gridtools::halo_descriptor(0,0,1,d3-1,d3);

	    //the following might be runtime value
	    uint_t total_time=3;

	    for (;final_step::current_time < total_time; ++final_step::current_time)
	    {
#ifdef CUDA_EXAMPLE
		// TODO: use placeholders here instead of the storage
		/*                                 component,snapshot */
		gridtools::boundary_apply_gpu< bc_periodic<0,0> >(halos, bc_periodic<0,0>()).apply(sol);
		gridtools::boundary_apply_gpu< bc_periodic<1,0> >(halos, bc_periodic<1,0>()).apply(sol);
#else
		// TODO: use placeholders here instead of the storage
		/*                             component,snapshot */
		gridtools::boundary_apply< bc_periodic<0,0> >(halos, bc_periodic<0,0>()).apply(sol);
		gridtools::boundary_apply< bc_periodic<1,0> >(halos, bc_periodic<1,0>()).apply(sol);
#endif
		shallow_water_stencil1->run();
                sol.print();
		shallow_water_stencil2->run();
		shallow_water_stencil->run();
	    }

            shallow_water_stencil1->finalize();
            shallow_water_stencil2->finalize();
            shallow_water_stencil->finalize();

            //tmpy.print();
            sol.print();
        }
        return true;

    }

}//namespace shallow_water
