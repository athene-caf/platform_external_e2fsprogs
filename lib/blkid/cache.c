/*
 * cache.c - allocation/initialization/free routines for cache
 *
 * Copyright (C) 2001 Andreas Dilger
 * Copyright (C) 2003 Theodore Ts'o
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 * %End-Header%
 */

#include <stdlib.h>
#include "blkidP.h"

#ifdef DEBUG_CACHE
#include <stdio.h>
#define DBG(x)	x
#else
#define DBG(x)
#endif

blkid_cache blkid_new_cache(void)
{
	blkid_cache cache;

	if (!(cache = (blkid_cache) calloc(1, sizeof(struct blkid_struct_cache))))
		return NULL;

	INIT_LIST_HEAD(&cache->bic_devs);
	INIT_LIST_HEAD(&cache->bic_tags);

	return cache;
}

void blkid_put_cache(blkid_cache cache)
{
	if (!cache)
		return;

	(void) blkid_flush_cache(cache);

	DBG(printf("freeing cache struct\n"));
	
	/* DEB_DUMP_CACHE(cache); */

	while (!list_empty(&cache->bic_devs)) {
		blkid_dev dev = list_entry(cache->bic_devs.next,
					   struct blkid_struct_dev,
					    bid_devs);
		blkid_free_dev(dev);
	}

	while (!list_empty(&cache->bic_tags)) {
		blkid_tag tag = list_entry(cache->bic_tags.next,
					   struct blkid_struct_tag,
					   bit_tags);

		while (!list_empty(&tag->bit_names)) {
			blkid_tag bad = list_entry(tag->bit_names.next,
						   struct blkid_struct_tag, 
						   bit_names);

			DBG(printf("warning: unfreed tag %s=%s\n",
				   bad->bit_name, bad->bit_val));
			blkid_free_tag(bad);
		}
		blkid_free_tag(tag);
	}
	if (cache->bic_filename)
		free(cache->bic_filename);
	
	free(cache);
}

#ifdef TEST_PROGRAM
int main(int argc, char** argv)
{
	blkid_cache cache = NULL;
	int ret;

	if ((argc > 2)) {
		fprintf(stderr, "Usage: %s [filename] \n", argv[0]);
		exit(1);
	}

	if ((ret = blkid_get_cache(&cache, argv[1])) < 0) {
		fprintf(stderr, "error %d parsing cache file %s\n", ret,
			argv[1] ? argv[1] : BLKID_CACHE_FILE);
		exit(1);
	}
	if ((cache = blkid_new_cache()) == NULL) {
		fprintf(stderr, "%s: error creating cache\n", argv[0]);
		exit(1);
	}
	if ((ret = blkid_probe_all(cache) < 0))
		fprintf(stderr, "error probing devices\n");

	blkid_put_cache(cache);

	return ret;
}
#endif
