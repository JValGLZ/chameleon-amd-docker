#include "XSbench_header.h"

#ifdef PAPI
#include <papi.h>

int EventSet_RAPL = PAPI_NULL;
int EventSet_PERF = PAPI_NULL;
long long rapl_values_before[2];  // RAPL counters
long long rapl_values_after[2];
long long perf_values_before[2];  // Performance counters (cycles, instructions only)
long long perf_values_after[2];
int rapl_initialized = 0;
int perf_initialized = 0;

int init_rapl_monitoring() {
    int retval;
    
    // Initialize PAPI library
    retval = PAPI_library_init(PAPI_VER_CURRENT);
    if (retval != PAPI_VER_CURRENT) {
        fprintf(stderr, "PAPI library init error!\n");
        return 1;
    }
    
    // Create RAPL event set
    EventSet_RAPL = PAPI_NULL;
    retval = PAPI_create_eventset(&EventSet_RAPL);
    if (retval != PAPI_OK) {
        fprintf(stderr, "Could not create RAPL event set\n");
        return 1;
    }
    
    // Add RAPL events
    if (PAPI_add_named_event(EventSet_RAPL, "rapl::RAPL_ENERGY_PKG:cpu=0") != PAPI_OK) {
        fprintf(stderr, "Could not add PACKAGE_ENERGY event\n");
        return 1;
    }
    if (PAPI_add_named_event(EventSet_RAPL, "rapl::RAPL_ENERGY_DRAM:cpu=0") != PAPI_OK) {
        fprintf(stderr, "Could not add DRAM_ENERGY event (may not exist on your CPU)\n");
        return 1;
    }
    rapl_initialized = 1;
    
    // Create Performance event set
    EventSet_PERF = PAPI_NULL;
    retval = PAPI_create_eventset(&EventSet_PERF);
    if (retval == PAPI_OK) {
        // Try to add performance counters (only the available ones)
        int perf_events_added = 0;
        
        printf("Attempting to add PAPI_TOT_CYC...\n");
        retval = PAPI_add_event(EventSet_PERF, PAPI_TOT_CYC);
        if (retval == PAPI_OK) {
            perf_events_added++;
            printf("Successfully added PAPI_TOT_CYC\n");
        } else {
            printf("Failed to add PAPI_TOT_CYC: %s\n", PAPI_strerror(retval));
        }
        
        printf("Attempting to add PAPI_TOT_INS...\n");
        retval = PAPI_add_event(EventSet_PERF, PAPI_TOT_INS);
        if (retval == PAPI_OK) {
            perf_events_added++;
            printf("Successfully added PAPI_TOT_INS\n");
        } else {
            printf("Failed to add PAPI_TOT_INS: %s\n", PAPI_strerror(retval));
        }
        
        if (perf_events_added == 2) {  // Both cycles and instructions
            perf_initialized = 1;
            printf("RAPL monitoring + Performance counters initialized successfully (%d events)\n", perf_events_added);
        } else {
            printf("RAPL monitoring initialized (insufficient performance counters: %d/2)\n", perf_events_added);
        }
    } else {
        printf("RAPL monitoring initialized (performance event set creation failed: %s)\n", PAPI_strerror(retval));
    }
    
    return 0;
}

void start_rapl_measurement() {
    // Start RAPL measurements
    if (rapl_initialized && PAPI_start(EventSet_RAPL) != PAPI_OK) {
        fprintf(stderr, "Could not start RAPL counters\n");
        return;
    }
    
    // Start Performance measurements
    if (perf_initialized && PAPI_start(EventSet_PERF) != PAPI_OK) {
        fprintf(stderr, "Could not start performance counters\n");
        return;
    }
    
    // Read initial values
    if (rapl_initialized && PAPI_read(EventSet_RAPL, rapl_values_before) != PAPI_OK) {
        fprintf(stderr, "Could not read RAPL counters\n");
        return;
    }
    
    if (perf_initialized && PAPI_read(EventSet_PERF, perf_values_before) != PAPI_OK) {
        fprintf(stderr, "Could not read performance counters\n");
        return;
    }
    
    printf("RAPL measurement started\n");
}

void stop_rapl_measurement() {
    // Read final values
    if (rapl_initialized && PAPI_read(EventSet_RAPL, rapl_values_after) != PAPI_OK) {
        fprintf(stderr, "Could not read RAPL counters\n");
        return;
    }
    
    if (perf_initialized && PAPI_read(EventSet_PERF, perf_values_after) != PAPI_OK) {
        fprintf(stderr, "Could not read performance counters\n");
        return;
    }
    
    // Stop counters
    if (rapl_initialized && PAPI_stop(EventSet_RAPL, rapl_values_after) != PAPI_OK) {
        fprintf(stderr, "Could not stop RAPL counters\n");
        return;
    }
    
    if (perf_initialized && PAPI_stop(EventSet_PERF, perf_values_after) != PAPI_OK) {
        fprintf(stderr, "Could not stop performance counters\n");
        return;
    }
    
    printf("RAPL measurement stopped\n");
}

void print_rapl_results(double runtime) {
    long long package_energy = 0, dram_energy = 0;
    long long total_cycles = 0, total_instructions = 0;
    
    // Calculate RAPL differences
    if (rapl_initialized) {
        package_energy = rapl_values_after[0] - rapl_values_before[0];
        dram_energy = rapl_values_after[1] - rapl_values_before[1];
    }
    
    // Calculate performance counter differences
    if (perf_initialized) {
        total_cycles = perf_values_after[0] - perf_values_before[0];
        total_instructions = perf_values_after[1] - perf_values_before[1];
    }
    
    printf("\n");
    printf("================================================================================\n");
    printf("                     ENERGY & PERFORMANCE RESULTS\n");
    printf("================================================================================\n");
    printf("Runtime: %.3f seconds\n", runtime);
    
    if (rapl_initialized) {
        printf("--------------------------------------------------------------------------------\n");
        printf("ENERGY MEASUREMENTS:\n");
        printf("Package Energy: %lld nJ (%.3f J)\n", package_energy, package_energy / 1e9);
        printf("DRAM Energy:    %lld nJ (%.3f J)\n", dram_energy, dram_energy / 1e9);
        printf("Total Energy:   %.3f J\n", (package_energy + dram_energy) / 1e9);
        
        if (runtime > 0) {
            printf("Average Package Power: %.3f W\n", (package_energy / 1e9) / runtime);
            printf("Average DRAM Power:    %.3f W\n", (dram_energy / 1e9) / runtime);
            printf("Average Total Power:   %.3f W\n", ((package_energy + dram_energy) / 1e9) / runtime);
        }
    }
    
    if (perf_initialized) {
        printf("--------------------------------------------------------------------------------\n");
        printf("PERFORMANCE COUNTERS:\n");
        printf("Total Cycles:       %lld\n", total_cycles);
        printf("Total Instructions: %lld\n", total_instructions);
        
        if (runtime > 0) {
            printf("Frequency:          %.3f GHz\n", (total_cycles / 1e9) / runtime);
            printf("IPS:                %.3f MIPS\n", (total_instructions / 1e6) / runtime);
        }
        
        if (total_cycles > 0) {
            printf("IPC:                %.3f instructions/cycle\n", (double)total_instructions / total_cycles);
        }
    }
    
    if (!rapl_initialized && !perf_initialized) {
        printf("No measurements available\n");
    }
    
    printf("================================================================================\n");
}

void cleanup_rapl_monitoring() {
    if (EventSet_RAPL != PAPI_NULL) {
        PAPI_destroy_eventset(&EventSet_RAPL);
    }
    if (EventSet_PERF != PAPI_NULL) {
        PAPI_destroy_eventset(&EventSet_PERF);
    }
    PAPI_shutdown();
    printf("RAPL monitoring cleaned up\n");
}

#endif
