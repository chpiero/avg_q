/*
 * Copyright (C) 1996-1999,2001,2003,2011 Bernd Feige
 * This file is part of avg_q and released under the GPL v3 (see avg_q/COPYING).
 */
 * 
 */
/*{{{}}}*/
/*{{{  Description*/
/*
 * differentiate is a simple transform method generating the derivative --
 * defined by the difference between two successive data points.
 * The first point of the result is set to 0.0.
 * The differentiation is done in place.
 * 						-- Bernd Feige 21.07.1993
 */
/*}}}  */

/*{{{  #includes*/
#include <stdlib.h>
#include <stdio.h>
#include "transform.h"
#include "bf.h"
/*}}}  */

enum ARGS_ENUM {
 ARGS_BYNAME=0,
 ARGS_ITEMPART, 
 NR_OF_ARGUMENTS
};
LOCAL transform_argument_descriptor argument_descriptors[NR_OF_ARGUMENTS]={
 {T_ARGS_TAKES_STRING_WORD, "channelnames: Restrict to these channel names", "n", ARGDESC_UNUSED, NULL},
 {T_ARGS_TAKES_LONG, "nr_of_item: work only on this item # (>=0)", "i", 0, NULL},
};
struct differentiate_storage {
 int *channel_list;
 Bool have_channel_list;
 int fromitem;
 int toitem;
};

/*{{{  differentiate_init(transform_info_ptr tinfo) {*/
METHODDEF void
differentiate_init(transform_info_ptr tinfo) {
 struct differentiate_storage *local_arg=(struct differentiate_storage *)tinfo->methods->local_storage;
 transform_argument *args=tinfo->methods->arguments;

 if (args[ARGS_BYNAME].is_set) {
  /* Note that this is NULL if no channel matched, which is why we need have_channel_list as well... */
  local_arg->channel_list=expand_channel_list(tinfo, args[ARGS_BYNAME].arg.s);
  local_arg->have_channel_list=TRUE;
 } else {
  local_arg->channel_list=NULL;
  local_arg->have_channel_list=FALSE;
 }

 local_arg->fromitem=0;
 local_arg->toitem=tinfo->itemsize-tinfo->leaveright-1;
 if (args[ARGS_ITEMPART].is_set) {
  local_arg->fromitem=local_arg->toitem=args[ARGS_ITEMPART].arg.i;
  if (local_arg->fromitem<0 || local_arg->fromitem>=tinfo->itemsize) {
   ERREXIT1(tinfo->emethods, "differentiate_init: No item number %d in file\n", MSGPARM(local_arg->fromitem));
  }
 }

 tinfo->methods->init_done=TRUE;
}
/*}}}  */

/*{{{  differentiate(transform_info_ptr tinfo) {*/
METHODDEF DATATYPE *
differentiate(transform_info_ptr tinfo) {
 struct differentiate_storage *local_arg=(struct differentiate_storage *)tinfo->methods->local_storage;
 DATATYPE previous, hold;
 transform_info_ptr const tinfoptr=tinfo;
 array myarray;

  int itempart;
  int shift, nrofshifts=1;
  DATATYPE *save_tsdata;

  tinfo_array(tinfoptr, &myarray);	/* The channels are the vectors */
  save_tsdata=myarray.start;

  if (tinfoptr->data_type==FREQ_DATA) {
   nrofshifts=tinfoptr->nrofshifts;
  }
  for (shift=0; shift<nrofshifts; shift++) {
   myarray.start=save_tsdata+shift*tinfo->nroffreq*tinfo->nr_of_channels*tinfo->itemsize;
   array_setreadwrite(&myarray);
   for (itempart=local_arg->fromitem; itempart<=local_arg->toitem; itempart++) {
    array_use_item(&myarray, itempart);
    do {
     if (local_arg->have_channel_list && !is_in_channellist(myarray.current_vector+1, local_arg->channel_list)) {
      array_nextvector(&myarray);
      continue;
     }
     /* Read the first element and leave it alone */
     previous=array_scan(&myarray);
     while (myarray.message==ARRAY_CONTINUE) {
      hold=READ_ELEMENT(&myarray);
      array_write(&myarray, hold-previous);
      previous=hold;
     }
    } while (myarray.message==ARRAY_ENDOFVECTOR);
   }
  }

 return tinfo->tsdata;
}
/*}}}  */

/*{{{  differentiate_exit(transform_info_ptr tinfo) {*/
METHODDEF void
differentiate_exit(transform_info_ptr tinfo) {
 struct differentiate_storage *local_arg=(struct differentiate_storage *)tinfo->methods->local_storage;

 free_pointer((void **)&local_arg->channel_list);

 tinfo->methods->init_done=FALSE;
}
/*}}}  */

/*{{{  select_differentiate(transform_info_ptr tinfo) {*/
GLOBAL void
select_differentiate(transform_info_ptr tinfo) {
 tinfo->methods->transform_init= &differentiate_init;
 tinfo->methods->transform= &differentiate;
 tinfo->methods->transform_exit= &differentiate_exit;
 tinfo->methods->method_type=TRANSFORM_METHOD;
 tinfo->methods->method_name="differentiate";
 tinfo->methods->method_description=
  "Transform method generating the derivative --\n"
  "defined by the difference between two successive data points.\n"
  "The first point is copied to the result unmodified.\n";
 tinfo->methods->local_storage_size=sizeof(struct differentiate_storage);
 tinfo->methods->nr_of_arguments=NR_OF_ARGUMENTS;
 tinfo->methods->argument_descriptors=argument_descriptors;
}
/*}}}  */
