#pragma once

#ifdef CUDA_EXAMPLE
#include <stencil-composition/backend_cuda.h>
#else
#include <stencil-composition/backend_host.h>
#endif
#include <storage/partitioner_trivial.h>
#include <storage/parallel_storage.h>
#include <stencil-composition/interval.h>
#include <stencil-composition/make_computation.h>
#include <communication/low-level/proc_grids_3D.h>
/** @file
    @brief This file shows an implementation of the "copy" stencil in parallel, simple copy of one field done on the backend*/

using gridtools::level;
using gridtools::arg_type;
using gridtools::range;
using gridtools::arg;

using namespace gridtools;
using namespace enumtype;


namespace copy_stencil{
// This is the definition of the special regions in the "vertical" direction
    typedef gridtools::interval<level<0,-1>, level<1,-1> > x_interval;
    typedef gridtools::interval<level<0,-2>, level<1,1> > axis;

// These are the stencil operators that compose the multistage stencil in this test
    struct copy_functor {
	typedef const arg_type<0>::type in;
	typedef arg_type<1>::type out;
	typedef boost::mpl::vector<in,out> arg_list;
	/* static const auto expression=in(1,0,0)-out(); */

	template <typename Evaluation>
	GT_FUNCTION
	static void Do(Evaluation const & eval, x_interval) {
		eval(out())
		=eval(in());
	}
    };

/*
 * The following operators and structs are for debugging only
 */
    std::ostream& operator<<(std::ostream& s, copy_functor const) {
	return s << "copy_functor";
    }

    bool test(uint_t d1, uint_t d2, uint_t d3) {

#ifdef CUDA_EXAMPLE
#define BACKEND backend<Cuda, Naive >
#else
#ifdef BACKEND_BLOCK
#define BACKEND backend<Host, Block >
#else
#define BACKEND backend<Host, Naive >
#endif
#endif
	//                   strides  1 x xy
	//                      dims  x y z
	typedef gridtools::layout_map<0,1,2> layout_t;
	typedef gridtools::BACKEND::storage_type<float_type, layout_t >::type storage_type;

    /* The nice interface does not compile today (CUDA 6.5) with nvcc (C++11 support not complete yet)*/

	// Definition of placeholders. The order of them reflect the order the user will deal with them
	// especially the non-temporary ones, in the construction of the domain
	typedef arg<0, storage_type> p_in;
	typedef arg<1, storage_type> p_out;
	// An array of placeholders to be passed to the domain
	// I'm using mpl::vector, but the final API should look slightly simpler
	typedef boost::mpl::vector<p_in, p_out> arg_type_list;
	/* typedef arg<1, vec_storage_type > p_out; */
        typedef storage_type::original_storage::pointer_type pointer_type;
	// Definition of the actual data fields that are used for input/output
//#ifdef CXX11_ENABLED
        MPI_3D_process_grid_t<gridtools::boollist<3> > comm(gridtools::boollist<3>(true,true,true), GCL_WORLD);
        ushort_t halo[3]={1,1,1};
        typedef partitioner_trivial<storage_type> partitioner_t;
        partitioner_t part(comm.ntasks(), comm.coordinates(), comm.dimensions(), halo);
 	parallel_storage<partitioner_t> in(part,d1,d2,d3);
 	parallel_storage<partitioner_t> out(part,d1,d2,d3);
        //in.initilaize(-1.);
        //out.initilaize(-2.);
	//storage_type in(d1,d2,d3);
	//storage_type out(d1,d2,d3);

	for(uint_t i=0; i<in.template dims<0>(); ++i)
	    for(uint_t j=0; j<in.template dims<1>(); ++j)
		for(uint_t k=0; k<in.template dims<2>(); ++k)
		{
		    in(i, j, k)=i+j+k;//+comm.coordinates()[0]*100+comm.coordinates()[1]*200+comm.coordinates()[2]*300;
		}

        std::cout<< "Halo descriptor 0 : [ " << part.template get_halo_descriptor<0>().minus() << ", "<< part.template get_halo_descriptor<0>().plus() << ", "<<part.template get_halo_descriptor<0>().begin() << ", "<<part.template get_halo_descriptor<0>().end() << ", "<<part.template get_halo_descriptor<0>().total_length()<<"];"<<std::endl;

        std::cout<< "Halo descriptor 1 : [ " << part.template get_halo_descriptor<1>().minus() << ", "<< part.template get_halo_descriptor<1>().plus() << ", "<<part.template get_halo_descriptor<1>().begin() << ", "<<part.template get_halo_descriptor<1>().end() << ", "<<part.template get_halo_descriptor<1>().total_length()<<"];"<<std::endl;
// // Definition of the physical dimensions of the problem.
	// The constructor takes the horizontal plane dimensions,
	// while the vertical ones are set according the the axis property soon after
        //gridtools::coordinates<axis> coords(part.template get_halo_descriptor<0>(), part.template get_halo_descriptor<1>());
        uint_t di[5] = {0, 0, 0, d1-1, d1};
        uint_t dj[5] = {0, 0, 0, d2-1, d2};
        gridtools::coordinates<axis> coords(di, dj);
        //k dimension not partitioned
	coords.value_list[0] = 0;
	coords.value_list[1] = d3-1;


        typedef gridtools::halo_exchange_dynamic_ut<gridtools::layout_map<0, 1, 2>,
                                                    gridtools::layout_map<0, 1, 2>,
                                                    pointer_type::pointee_t, 3,
#ifdef CUDA_EXAMPLE
            gridtools::gcl_gpu,
#else
            gridtools::gcl_cpu,
#endif
            gridtools::version_manual> pattern_type;

        pattern_type he(pattern_type::grid_type::period_type(true, true, true), comm.communicator());
        printf("halo exchange ok\n");

        he.add_halo<0>(part.template get_halo_descriptor<0>());
        he.add_halo<1>(part.template get_halo_descriptor<1>());
        he.add_halo<2>(0, 0, 0, d3 - 1, d3);

        he.setup(2);
        printf("halo set up\n");


	// construction of the domain. The domain is the physical domain of the problem, with all the physical fields that are used, temporary and not
	// It must be noted that the only fields to be passed to the constructor are the non-temporary.
	// The order in which they have to be passed is the order in which they appear scanning the placeholders in order. (I don't particularly like this)
	gridtools::domain_type<arg_type_list> domain
	    (boost::fusion::make_vector(&in, &out));

	/*
	  Here we do lot of stuff
	  1) We pass to the intermediate representation ::run function the description
	  of the stencil, which is a multi-stage stencil (mss)
	  The mss includes (in order of execution) a laplacian, two fluxes which are independent
	  and a final step that is the out_function
	  2) The logical physical domain with the fields to use
	  3) The actual domain dimensions
	*/

// \todo simplify the following using the auto keyword from C++11
#ifdef __CUDACC__
	gridtools::computation* copy =
#else
	    boost::shared_ptr<gridtools::computation> copy =
#endif
	    gridtools::make_computation<gridtools::BACKEND, layout_t>
	    (
		gridtools::make_mss // mss_descriptor
		(
		    execute<forward>(),
		    gridtools::make_esf<copy_functor>(p_in() // esf_descriptor
                                                      , p_out()
			)
		    ),
		domain, coords
		);
        printf("computation instantiated\n");

	copy->ready();

        printf("computation ready\n");

	copy->steady();

        printf("computation steady\n");

	copy->run();

        printf("computation run\n");

	copy->finalize();

        printf("computation finalized\n");

	std::vector<pointer_type::pointee_t*> vec(2);
        vec[0]=in.fields()[0].get();
        vec[1]=out.fields()[0].get();

	he.pack(vec);

        printf("copy packed \n");

	he.exchange();

        printf("copy exchanged\n");

	he.unpack(vec);

        printf("copy unpacked\n");

        in.print();

 	MPI_Barrier(GCL_WORLD);
 	GCL_Finalize();

	return true;
    }

}//namespace copy_stencil