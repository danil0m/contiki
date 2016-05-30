/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         RSSI Objective Function (MRHOF)
 *
 *        This implementation uses RSSI as metric
 *
 *
 * \author Danilo Martino <danilo.martino17@gmail.com>
 */

/**
 * \addtogroup uip6
 * @{
 */
#include "contiki.h"
#include "dev/radio-sensor.h"
#include "net/rpl/rpl-private.h"
#include "net/nbr-table.h"

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

/* Reject parents that have a higher link metric than the following. */
#define MAX_LINK_METRIC			10

/* Reject parents that have a higher path cost than the following. */
#define MAX_PATH_COST			100
/*The theshold of the radio when PER begin to lower*/
#define RSSI_THRESHOLD			-95

#define SIGMA					2.5
#define RADIO_THRESHOLD       -101

static void reset(rpl_dag_t *);
static void neighbor_link_callback(rpl_parent_t *, int, int);
static rpl_parent_t *best_parent(rpl_parent_t *, rpl_parent_t *);
static rpl_dag_t *best_dag(rpl_dag_t *, rpl_dag_t *);
static rpl_rank_t calculate_rank(rpl_parent_t *, rpl_rank_t);
static void update_metric_container(rpl_instance_t *);

rpl_of_t rpl_rssiof = {
  reset,
  neighbor_link_callback,
  best_parent,
  best_dag,
  calculate_rank,
  update_metric_container,
  2
};

typedef uint16_t rpl_path_metric_t;

static rpl_path_metric_t
calculate_path_metric(rpl_parent_t *p)
{
  uip_ds6_nbr_t *nbr;
  if(p == NULL) {
    return MAX_PATH_COST * RPL_DAG_MC_ETX_DIVISOR;
  }
  nbr = rpl_get_nbr(p);
  if(nbr == NULL) {
    return MAX_PATH_COST * RPL_DAG_MC_ETX_DIVISOR;
  }
    return p->rank + (uint16_t)nbr->link_metric;
}
static void
reset(rpl_dag_t *dag)
{
  PRINTF("RPL: Reset RSSIOF\n");
}
static void
neighbor_link_callback(rpl_parent_t *p, int status, int numtx)
{
	  uip_ds6_nbr_t *nbr = NULL;
	  int rssi_val;
	 nbr = rpl_get_nbr(p);
	  if(nbr == NULL) {
	    /* No neighbor for this parent - something bad has occurred */
	    return;
	  }
	  rssi_val= radio_sensor.value(RADIO_SENSOR_LAST_PACKET);
	  if(rssi_val<=RSSI_THRESHOLD*10){
	    nbr->link_metric =1*RPL_DAG_MC_ETX_DIVISOR;
	  }else if (rssi_val>RSSI_THRESHOLD*10 &&rssi_val<=(RSSI_THRESHOLD +SIGMA)*10){
		  nbr->link_metric =(uint16_t)(10*RPL_DAG_MC_ETX_DIVISOR*MAX_LINK_METRIC/(RADIO_THRESHOLD-RSSI_THRESHOLD)*(rssi_val-RSSI_THRESHOLD*10));
	  }else {
		  nbr->link_metric=MAX_LINK_METRIC*RPL_DAG_MC_ETX_DIVISOR;
	  }

}


static rpl_rank_t
calculate_rank(rpl_parent_t *p, rpl_rank_t base_rank)
{
  rpl_rank_t new_rank;
  rpl_rank_t rank_increase;
  uip_ds6_nbr_t *nbr;

  if(p == NULL || (nbr = rpl_get_nbr(p)) == NULL) {
    if(base_rank == 0) {
      return INFINITE_RANK;
    }
    rank_increase = RPL_INIT_LINK_METRIC * RPL_DAG_MC_ETX_DIVISOR;
  } else {
    rank_increase = nbr->link_metric;
    if(base_rank == 0) {
      base_rank = p->rank;
    }
  }

  if(INFINITE_RANK - base_rank < rank_increase) {
    /* Reached the maximum rank. */
    new_rank = INFINITE_RANK;
  } else {
   /* Calculate the rank based on the new rank information from DIO or
      stored otherwise. */
    new_rank = base_rank + rank_increase;
  }

  return new_rank;
}

static rpl_dag_t *
best_dag(rpl_dag_t *d1, rpl_dag_t *d2)
{
  if(d1->grounded != d2->grounded) {
    return d1->grounded ? d1 : d2;
  }

  if(d1->preference != d2->preference) {
    return d1->preference > d2->preference ? d1 : d2;
  }

  return d1->rank < d2->rank ? d1 : d2;
}

static rpl_parent_t *
best_parent(rpl_parent_t *p1, rpl_parent_t *p2)
{
  rpl_dag_t *dag;
  rpl_path_metric_t p1_metric;
  rpl_path_metric_t p2_metric;
  uint16_t min_diff=2*RPL_DAG_MC_ETX_DIVISOR;
  dag = p1->dag; /* Both parents are in the same DAG. */
  p1_metric = calculate_path_metric(p1);
  p2_metric = calculate_path_metric(p2);
  if(p1 == dag->preferred_parent || p2 == dag->preferred_parent) {
      if(p1_metric <= p2_metric + min_diff &&
         p1_metric >= p2_metric - min_diff) {
        PRINTF("RPL: RSSIOF hysteresis: %u <= %u <= %u\n",
               p2_metric - min_diff,
               p1_metric,
               p2_metric + min_diff);
        return dag->preferred_parent;
      }
    }



  return p1_metric < p2_metric ? p1 : p2;

}

static void
update_metric_container(rpl_instance_t *instance)
{
  instance->mc.type = RPL_DAG_MC;
}

/** @}*/
