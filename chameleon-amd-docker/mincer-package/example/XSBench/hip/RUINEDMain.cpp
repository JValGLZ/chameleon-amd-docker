#include "XSbench_header.h"

int main( int argc, char* argv[] )
{
	// =====================================================================
	// Initialization & Command Line Read-In
	// =====================================================================
	int version = 20;
	int mype = 0;
	double omp_start, omp_end;
	int nprocs = 1;
	unsigned long long verification;

	// Process CLI Fields -- store in "Inputs" structure
	Inputs in = read_CLI( argc, argv );

	// Print-out of Input Summary
	if( mype == 0 )
		print_inputs( in, nprocs, version );

	// =====================================================================
	// Prepare Nuclide Energy Grids, Unionized Energy Grid, & Material Data
	// This is not reflective of a real Monte Carlo simulation workload,
	// therefore, do not profile this region!
	// =====================================================================
	
	SimulationData SD;

	// If read from file mode is selected, skip initialization and load
	// all simulation data structures from file instead
	if( in.binary_mode == READ )
		SD = binary_read(in);
	else
		SD = grid_init_do_not_profile( in, mype );

	// If writing from file mode is selected, write all simulation data
	// structures to file
	if( in.binary_mode == WRITE && mype == 0 )
		binary_write(in, SD);

	// Move data to GPU
	SimulationData GSD = move_simulation_data_to_device( in, mype, SD );

	// =====================================================================
	// Cross Section (XS) Parallel Lookup Simulation
	// This is the section that should be profiled, as it reflects a 
	// realistic continuous energy Monte Carlo macroscopic cross section
	// lookup kernel.
	// =====================================================================
	
	// Initialize PAPI
	int papi_retval;
	int EventSet = PAPI_NULL;
	long long papi_values[8];  // Increased array size for more events (including FP64)
	int num_events = 0;
	bool has_busy_percent = false;
	bool has_power = false;
	bool has_temperature = false;
	bool has_fp_ops = false;
	bool has_fp64_ops = false;
	bool has_instructions = false;
	
	// Initialize PAPI library
	papi_retval = PAPI_library_init(PAPI_VER_CURRENT);
	if (papi_retval != PAPI_VER_CURRENT && papi_retval > 0) {
		printf("PAPI library version mismatch!\n");
	} else if (papi_retval < 0) {
		printf("PAPI library initialization error: %s\n", PAPI_strerror(papi_retval));
	} else {
		printf("PAPI library initialized successfully\n");
		
		// Create event set
		papi_retval = PAPI_create_eventset(&EventSet);
		if (papi_retval != PAPI_OK) {
			printf("PAPI create eventset error: %s\n", PAPI_strerror(papi_retval));
		} else {
			// Try to add GPU utilization event
			papi_retval = PAPI_add_named_event(EventSet, "rocm_smi:::busy_percent:device=0");
			if (papi_retval == PAPI_OK) {
				printf("Added ROCm SMI GPU busy percent event\n");
				has_busy_percent = true;
				num_events++;
			} else {
				printf("Could not add GPU busy percent event: %s\n", PAPI_strerror(papi_retval));
			}
			
			// Try to add GPU power monitoring
			papi_retval = PAPI_add_named_event(EventSet, "rocm_smi:::power_average:device=0:sensor=0");
			if (papi_retval == PAPI_OK) {
				printf("Added ROCm SMI power monitoring event\n");
				has_power = true;
				num_events++;
			} else {
				printf("Could not add power monitoring event: %s\n", PAPI_strerror(papi_retval));
			}
			
			// Try to add GPU temperature monitoring
			papi_retval = PAPI_add_named_event(EventSet, "rocm_smi:::temp_current:device=0:sensor=0");
			if (papi_retval == PAPI_OK) {
				printf("Added ROCm SMI temperature monitoring event\n");
				has_temperature = true;
				num_events++;
			} else {
				printf("Could not add temperature monitoring event: %s\n", PAPI_strerror(papi_retval));
			}
			
			// Try to add GPU VALU instructions (without device specification)
			papi_retval = PAPI_add_named_event(EventSet, "rocm:::VALUInsts");
			if (papi_retval == PAPI_OK) {
				printf("Added ROCm VALU instructions event\n");
				has_fp64_ops = true;
				num_events++;
			} else {
				printf("Could not add VALU instructions event: %s\n", PAPI_strerror(papi_retval));
				// Try alternative event
				papi_retval = PAPI_add_named_event(EventSet, "rocm:::SQ_INSTS_VALU");
				if (papi_retval == PAPI_OK) {
					printf("Added ROCm SQ_INSTS_VALU event\n");
					has_fp64_ops = true;
					num_events++;
				} else {
					printf("Could not add SQ_INSTS_VALU event: %s\n", PAPI_strerror(papi_retval));
				}
			}
			
			// Try to add GPU VALU busy cycles
			papi_retval = PAPI_add_named_event(EventSet, "rocm:::VALUBusy");
			if (papi_retval == PAPI_OK) {
				printf("Added ROCm VALU busy event\n");
				has_fp_ops = true;
				num_events++;
			} else {
				printf("Could not add VALU busy event: %s\n", PAPI_strerror(papi_retval));
				// Try alternative
				papi_retval = PAPI_add_named_event(EventSet, "rocm:::SQ_WAVES");
				if (papi_retval == PAPI_OK) {
					printf("Added ROCm SQ_WAVES event\n");
					has_fp_ops = true;
					num_events++;
				} else {
					printf("Could not add SQ_WAVES event: %s\n", PAPI_strerror(papi_retval));
				}
			}
			
			if (num_events > 0) {
				// Start monitoring
				papi_retval = PAPI_start(EventSet);
				if (papi_retval != PAPI_OK) {
					printf("PAPI start error: %s\n", PAPI_strerror(papi_retval));
					num_events = 0;
				} else {
					printf("Started PAPI monitoring with %d events\n", num_events);
				}
			}
		}
	}

	if( mype == 0 )
	{
		printf("\n");
		border_print();
		center_print("SIMULATION", 79);
		border_print();
	}

	// Start Simulation Timer
	omp_start = get_time();

	// Run simulation
	if( in.simulation_method == EVENT_BASED )
	{
		if( in.kernel_id == 0 )
			verification = run_event_based_simulation_baseline(in, GSD, mype);
/*
		else if( in.kernel_id == 1 )
			verification = run_event_based_simulation_optimization_1(in, GSD, mype);
		else if( in.kernel_id == 2 )
			verification = run_event_based_simulation_optimization_2(in, GSD, mype);
		else if( in.kernel_id == 3 )
			verification = run_event_based_simulation_optimization_3(in, GSD, mype);
		else if( in.kernel_id == 4 )
			verification = run_event_based_simulation_optimization_4(in, GSD, mype);
		else if( in.kernel_id == 5 )
			verification = run_event_based_simulation_optimization_5(in, GSD, mype);
		else if( in.kernel_id == 6 )
			verification = run_event_based_simulation_optimization_6(in, GSD, mype);
*/
		else
		{
			printf("Error: No kernel ID %d found!\n", in.kernel_id);
			exit(1);
		}
	}
	else
	{
		printf("History-based simulation not implemented in CUDA code. Instead,\nuse the event-based method with \"-m event\" argument.\n");
		exit(1);
	}

	if( mype == 0)	
	{	
		printf("\n" );
		printf("Simulation complete.\n" );
	}

	// End Simulation Timer
	omp_end = get_time();
	
	// Stop PAPI monitoring and read results
	if (num_events > 0) {
		papi_retval = PAPI_stop(EventSet, papi_values);
		if (papi_retval != PAPI_OK) {
			printf("PAPI stop error: %s\n", PAPI_strerror(papi_retval));
		} else {
			printf("\n");
			border_print();
			center_print("PAPI GPU MONITORING RESULTS", 79);
			border_print();
			
			// Print results for each event that was successfully added
			int event_idx = 0;
			
			if (has_busy_percent) {
				printf("GPU Busy Percent: %lld%%\n", papi_values[event_idx]);
				event_idx++;
			}
			
			if (has_power) {
				printf("GPU Power Usage: %lld watts\n", papi_values[event_idx] / 1000000); // Convert from microwatts
				event_idx++;
			}
			
			if (has_temperature) {
				printf("GPU Temperature: %lld°C\n", papi_values[event_idx] / 1000); // Convert from millidegrees
				event_idx++;
			}
			
			if (has_fp64_ops) {
				printf("GPU VALU/FP Instructions: %lld\n", papi_values[event_idx]);
				event_idx++;
			}
			
			if (has_fp_ops) {
				printf("GPU VALU Busy Cycles/Waves: %lld\n", papi_values[event_idx]);
				event_idx++;
			}
			
			border_print();
		}
		
		// Clean up PAPI
		PAPI_cleanup_eventset(EventSet);
		PAPI_destroy_eventset(&EventSet);
	}
	
	PAPI_shutdown();

	// Final Hash Step
	verification = verification % 999983;

	// Print / Save Results and Exit
	int is_invalid_result = print_results( in, mype, omp_end-omp_start, nprocs, verification );

	return is_invalid_result;
}
