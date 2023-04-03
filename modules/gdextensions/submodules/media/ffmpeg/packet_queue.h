/**************************************************************************/
/*  packet_queue.h                                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H

#include "core/os/memory.h"

#include <libavformat/avformat.h>

typedef struct PacketQueue {
	AVPacketList *first_pkt, *last_pkt;
	int nb_packets;
	int size;
} PacketQueue;

PacketQueue *packet_queue_init() {
	PacketQueue *q;
	q = (PacketQueue *)memalloc(sizeof(PacketQueue));
	if (q != nullptr) {
		memset(q, 0, sizeof(PacketQueue));
	}
	return q;
}

void packet_queue_flush(PacketQueue *q) {
	AVPacketList *pkt, *pkt1;

	for (pkt = q->first_pkt; pkt; pkt = pkt1) {
		pkt1 = pkt->next;
		av_packet_unref(&pkt->pkt);
		memfree(pkt);
	}
	q->last_pkt = nullptr;
	q->first_pkt = nullptr;
	q->nb_packets = 0;
	q->size = 0;
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt) {
	AVPacketList *pkt1;
	pkt1 = (AVPacketList *)memalloc(sizeof(AVPacketList));
	if (!pkt1)
		return -1;
	memcpy(&pkt1->pkt, pkt, sizeof(AVPacket)); // pkt1->pkt = *pkt;
	pkt1->next = nullptr;

	if (!q->last_pkt)
		q->first_pkt = pkt1;
	else
		q->last_pkt->next = pkt1;
	q->last_pkt = pkt1;
	q->nb_packets++;
	q->size += pkt1->pkt.size;
	return 0;
}

int packet_queue_get(PacketQueue *q, AVPacket *pkt) {
	AVPacketList *pkt1;

	pkt1 = q->first_pkt;
	if (pkt1) {
		q->first_pkt = pkt1->next;
		if (!q->first_pkt)
			q->last_pkt = nullptr;
		q->nb_packets--;
		q->size -= pkt1->pkt.size;
		memcpy(pkt, &pkt1->pkt, sizeof(AVPacket)); // *pkt = pkt1->pkt;
		memfree(pkt1);
		return 1;
	} else {
		return 0;
	}
}

void packet_queue_deinit(PacketQueue *q) {
	AVPacket pt;
	while (packet_queue_get(q, &pt)) {
		av_packet_unref(&pt);
	}
	memfree(q);
}

#endif /* PACKET_QUEUE_H */
