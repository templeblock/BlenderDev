/**
 * $Id: node_exec.c 39944 2011-09-05 22:04:30Z gsrb3d $
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2007 Blender Foundation.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): Nathan Letwory.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/nodes/intern/node_exec.c
 *  \ingroup nodes
 */


#include "DNA_node_types.h"

#include "BLI_listbase.h"
#include "BLI_math.h"
#include "BLI_utildefines.h"

#include "BKE_node.h"

#include "MEM_guardedalloc.h"

#include "node_exec.h"


/* for a given socket, find the actual stack entry */
bNodeStack *node_get_socket_stack(bNodeStack *stack, bNodeSocket *sock)
{
	return stack + sock->stack_index;
}

void node_get_stack(bNode *node, bNodeStack *stack, bNodeStack **in, bNodeStack **out)
{
	bNodeSocket *sock;
	
	/* build pointer stack */
	if (in) {
		for(sock= node->inputs.first; sock; sock= sock->next) {
			*(in++) = node_get_socket_stack(stack, sock);
		}
	}
	
	if (out) {
		for(sock= node->outputs.first; sock; sock= sock->next) {
			*(out++) = node_get_socket_stack(stack, sock);
		}
	}
}

void node_init_input_index(bNodeSocket *sock, int *index)
{
	if (sock->link && sock->link->fromsock) {
		sock->stack_index = sock->link->fromsock->stack_index;
	}
	else {
		sock->stack_index = (*index)++;
	}
}

void node_init_output_index(bNodeSocket *sock, int *index)
{
	sock->stack_index = (*index)++;
}

/* basic preparation of socket stacks */
static struct bNodeStack *setup_stack(bNodeStack *stack, bNodeSocket *sock)
{
	bNodeStack *ns = node_get_socket_stack(stack, sock);
	float null_value[4]= {0.0f, 0.0f, 0.0f, 0.0f};
	
	/* don't mess with remote socket stacks, these are initialized by other nodes! */
	if (sock->link)
		return ns;
	
	ns->sockettype = sock->type;
	
	if (sock->default_value) {
		switch (sock->type) {
		case SOCK_FLOAT:
			ns->vec[0] = ((bNodeSocketValueFloat*)sock->default_value)->value;
			break;
		case SOCK_VECTOR:
			copy_v3_v3(ns->vec, ((bNodeSocketValueVector*)sock->default_value)->value);
			break;
		case SOCK_RGBA:
			copy_v4_v4(ns->vec, ((bNodeSocketValueRGBA*)sock->default_value)->value);
			break;
		}
	}
	else {
		switch (sock->type) {
		case SOCK_FLOAT:
			ns->vec[0] = 0.0f;
			break;
		case SOCK_VECTOR:
			copy_v3_v3(ns->vec, null_value);
			break;
		case SOCK_RGBA:
			copy_v4_v4(ns->vec, null_value);
			break;
		}
	}
	
	return ns;
}

bNodeTreeExec *ntree_exec_begin(bNodeTree *ntree)
{
	bNodeTreeExec *exec;
	bNode *node;
	bNodeExec *nodeexec;
	bNodeSocket *sock, *gsock;
	bNodeStack *ns;
	int index= 0;
	bNode **nodelist;
	int totnodes, n;
	
	if((ntree->init & NTREE_TYPE_INIT)==0)
		ntreeInitTypes(ntree);
	
	/* get a dependency-sorted list of nodes */
	ntreeGetDependencyList(ntree, &nodelist, &totnodes);
	
	/* XXX could let callbacks do this for specialized data */
	exec = MEM_callocN(sizeof(bNodeTreeExec), "node tree execution data");
	/* backpointer to node tree */
	exec->nodetree = ntree;
	
	/* group inputs essentially work as outputs */
	for(gsock=ntree->inputs.first; gsock; gsock = gsock->next)
		node_init_output_index(gsock, &index);
	/* set stack indexes */
	for(n=0; n < totnodes; ++n) {
		node = nodelist[n];
		
		node->stack_index = index;
		
		/* init node socket stack indexes */
		for (sock=node->inputs.first; sock; sock=sock->next)
			node_init_input_index(sock, &index);
		for (sock=node->outputs.first; sock; sock=sock->next)
			node_init_output_index(sock, &index);
	}
	/* group outputs essentially work as inputs */
	for(gsock=ntree->outputs.first; gsock; gsock = gsock->next)
		node_init_input_index(gsock, &index);
	
	/* allocated exec data pointers for nodes */
	exec->totnodes = totnodes;
	exec->nodeexec = MEM_callocN(exec->totnodes * sizeof(bNodeExec), "node execution data");
	/* allocate data pointer for node stack */
	exec->stacksize = index;
	exec->stack = MEM_callocN(exec->stacksize * sizeof(bNodeStack), "bNodeStack");
	
	/* prepare group tree inputs */
	for (sock=ntree->inputs.first; sock; sock=sock->next) {
		ns = setup_stack(exec->stack, sock);
		if (ns->hasoutput)
			ns->hasinput = 1;
	}
	/* prepare all internal nodes for execution */
	for(n=0, nodeexec= exec->nodeexec; n < totnodes; ++n, ++nodeexec) {
		node = nodeexec->node = nodelist[n];
		
		/* tag inputs */
		for (sock=node->inputs.first; sock; sock=sock->next) {
			/* disable the node if an input link is invalid */
			if(sock->link && !(sock->link->flag & NODE_LINK_VALID))
				node->need_exec= 0;
			
			ns = setup_stack(exec->stack, sock);
			if (ns->hasoutput)
				ns->hasinput = 1;
		}
		
		/* tag all outputs */
		for (sock=node->outputs.first; sock; sock=sock->next) {
			ns = setup_stack(exec->stack, sock);
			ns->hasoutput = 1;
		}
		
		if(node->typeinfo->initexecfunc)
			nodeexec->data = node->typeinfo->initexecfunc(node);
	}
	/* prepare group tree outputs */
	for (sock=ntree->outputs.first; sock; sock=sock->next) {
		ns = setup_stack(exec->stack, sock);
		ns->hasoutput = 1;
	}
	
	if (nodelist)
		MEM_freeN(nodelist);
	
	return exec;
}

void ntree_exec_end(bNodeTreeExec *exec)
{
	bNodeExec *nodeexec;
	int n;
	
	if (exec->stack)
		MEM_freeN(exec->stack);
	
	for(n=0, nodeexec= exec->nodeexec; n < exec->totnodes; ++n, ++nodeexec) {
		if (nodeexec->node->typeinfo->freeexecfunc)
			nodeexec->node->typeinfo->freeexecfunc(nodeexec->node, nodeexec->data);
	}
	
	if (exec->nodeexec)
		MEM_freeN(exec->nodeexec);
	
	MEM_freeN(exec);
}

/**** Compositor/Material/Texture trees ****/

bNodeThreadStack *ntreeGetThreadStack(bNodeTreeExec *exec, int thread)
{
	ListBase *lb= &exec->threadstack[thread];
	bNodeThreadStack *nts;
	
	for(nts=lb->first; nts; nts=nts->next) {
		if(!nts->used) {
			nts->used= 1;
			break;
		}
	}
	
	if (!nts) {
		nts= MEM_callocN(sizeof(bNodeThreadStack), "bNodeThreadStack");
		nts->stack= MEM_dupallocN(exec->stack);
		nts->used= 1;
		BLI_addtail(lb, nts);
	}

	return nts;
}

void ntreeReleaseThreadStack(bNodeThreadStack *nts)
{
	nts->used = 0;
}

void ntreeExecNodes(bNodeTreeExec *exec, void *callerdata, int thread)
{
	bNodeStack *nsin[MAX_SOCKET];	/* arbitrary... watch this */
	bNodeStack *nsout[MAX_SOCKET];	/* arbitrary... watch this */
	bNodeExec *nodeexec;
	bNode *node;
	int n;
	
	/* nodes are presorted, so exec is in order of list */
	
	for(n=0, nodeexec= exec->nodeexec; n < exec->totnodes; ++n, ++nodeexec) {
		node = nodeexec->node;
		if(node->need_exec) {
			node_get_stack(node, exec->stack, nsin, nsout);
			if(node->typeinfo->execfunc)
				node->typeinfo->execfunc(callerdata, node, nsin, nsout);
			else if (node->typeinfo->newexecfunc)
				node->typeinfo->newexecfunc(callerdata, thread, node, nodeexec->data, nsin, nsout);
		}
	}
}

void ntreeExecThreadNodes(bNodeTreeExec *exec, bNodeThreadStack *nts, void *callerdata, int thread)
{
	bNodeStack *nsin[MAX_SOCKET];	/* arbitrary... watch this */
	bNodeStack *nsout[MAX_SOCKET];	/* arbitrary... watch this */
	bNodeExec *nodeexec;
	bNode *node;
	int n;
	
	/* nodes are presorted, so exec is in order of list */
	
	for(n=0, nodeexec= exec->nodeexec; n < exec->totnodes; ++n, ++nodeexec) {
		node = nodeexec->node;
		if(node->need_exec) {
			node_get_stack(node, nts->stack, nsin, nsout);
			if(node->typeinfo->execfunc)
				node->typeinfo->execfunc(callerdata, node, nsin, nsout);
			else if (node->typeinfo->newexecfunc)
				node->typeinfo->newexecfunc(callerdata, thread, node, nodeexec->data, nsin, nsout);
		}
	}
}
