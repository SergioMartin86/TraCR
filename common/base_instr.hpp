/*
 * Copyright Huawei Technologies Switzerland AG
 * All rights reserved.
 */

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>		 // For getpid()
#include <ovni.h>
#include "compat.h"
#include "common.h"

/**
 * Function taken from here:
 * https://ovni.readthedocs.io/en/master/user/runtime/#setup_metadata
 */
static inline void 
thread_execute(int32_t cpu, int32_t ctid, uint64_t tag)
{
    struct ovni_ev ev = {0};
    ovni_ev_set_clock(&ev, ovni_clock_now());
    ovni_ev_set_mcv(&ev, "OHx");
    ovni_payload_add(&ev, (uint8_t *) &cpu, sizeof(cpu));
    ovni_payload_add(&ev, (uint8_t *) &ctid, sizeof(ctid));
    ovni_payload_add(&ev, (uint8_t *) &tag, sizeof(tag));
    ovni_ev_emit(&ev);
}


/**
 * 
 */
static inline void
instrumentation_init_proc()
{
    char hostname[OVNI_MAX_HOSTNAME];

	if (gethostname(hostname, OVNI_MAX_HOSTNAME) != 0)
		die("gethostname failed");

	ovni_version_check();
	ovni_proc_init(1, hostname, getpid());
}

/**
 * 
 */
static inline void
instrumentation_init_thread(int rank)
{
	ovni_thread_init(get_tid());

    ovni_add_cpu(rank, rank);

	dbg("thread %d has pid %d and cpu %d\n",
			get_tid(), getpid(), rank);

	ovni_thread_require("ovni", "1.1.0");
	thread_execute(rank, -1, 0);
}

/**
 * 
 */
static inline void
instrumentation_thread_end(void)
{
	struct ovni_ev ev = {0};

	ovni_ev_set_mcv(&ev, "OHe");
	ovni_ev_set_clock(&ev, (uint64_t) ovni_clock_now());
	ovni_ev_emit(&ev);

	/* Flush the events to disk before killing the thread */
	ovni_flush();
}

/**
 * 
 */
static inline void
instrumentation_end(void)
{
	instrumentation_thread_end();
	ovni_thread_free();
	ovni_proc_fini();
}

