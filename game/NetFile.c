
#include "NetFile.h"

#include "egoboo_utility.h"
#include "egoboo_strutil.h"

#include <assert.h>
#include <time.h>


struct nfile_SendState_t;
struct nfile_ReceiveState_t;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
// the nfile module info

static bool_t    _nfile_atexit_registered = bfalse;
static NetHost * _nfile_host = NULL;

static retval_t  _nfile_Initialize(void);
static void      _nfile_Quit(void);
static retval_t  _nfile_startUp(void);
static retval_t  _nfhost_shutDown(void);
static retval_t  _nfile_receiveInitialize(void);
static retval_t  _nfile_sendInitialize(void);


//--------------------------------------------------------------------------------------------
// NFileState private initialization
static NFileState * NFileState_new(NFileState * nfs, NetState * ns);
static bool_t       NFileState_delete(NFileState * nfs);

//--------------------------------------------------------------------------------------------
static int nfhost_HostCallback(void *);
static retval_t nfhost_startThreads();
static retval_t nfhost_stopThreads();

//--------------------------------------------------------------------------------------------
static struct nfile_SendState_t * _nfile_snd  = NULL;
struct nfile_SendState_t * _nfile_getSend();
static retval_t           _nfile_Send_initialize();
static int                _nfile_sendCallback(void * snd);

static struct nfile_ReceiveState_t * _nfile_rec  = NULL;
struct nfile_ReceiveState_t * _nfile_getReceive();
static retval_t              _nfile_Receive_initialize();
static int                   _nfile_receiveCallback(void * nfs);

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

// File transfer variables & structures
typedef struct nfile_SendInfo_t
{
  //NetState    * ns;
  ENetHost    * host;
  ENetPeer    * target;

  char      sourceName[NET_MAX_FILE_NAME];
  char      destName[NET_MAX_FILE_NAME];
} nfile_SendInfo;

//--------------------------------------------------------------------------------------------
// File transfer queue
typedef struct nfile_SendQueue_t
{
  bool_t initialized;

  int numTransfers;
  nfile_SendInfo transferStates[NET_MAX_FILE_TRANSFERS];

  int TransferHead; // Queue indices
  int TransferTail;

} nfile_SendQueue;

nfile_SendQueue * nfile_SendQueue_new(nfile_SendQueue * q);
bool_t            nfile_SendQueue_delete(nfile_SendQueue * q);
nfile_SendQueue * nfile_SendQueue_renew(nfile_SendQueue * q);


retval_t nfile_SendQueue_add(NFileState * ns, ENetAddress * target_address, char *source, char *dest);

//--------------------------------------------------------------------------------------------

// File transfer variables & structures
typedef struct nfile_ReceiveInfo_t
{
  bool_t     allocated;

  Uint32     event_type;
  ENetPeer * sender;
  Uint8    * buffer;
  char       destName[NET_MAX_FILE_NAME];

  size_t     buffer_size;
} nfile_ReceiveInfo;

static bool_t nfile_ReceiveInfo_alloc( nfile_ReceiveInfo * nri, size_t size);
static bool_t nfile_ReceiveInfo_dealloc( nfile_ReceiveInfo * nri);

// File transfer queue
typedef struct nfile_ReceiveQueue_t
{
  bool_t initialized;

  int numTransfers;
  nfile_ReceiveInfo transferStates[NET_MAX_FILE_TRANSFERS];

  int TransferHead; // Queue indices
  int TransferTail;

} nfile_ReceiveQueue;

nfile_ReceiveQueue * nfile_ReceiveQueue_new(nfile_ReceiveQueue * q);
bool_t               nfile_ReceiveQueue_delete(nfile_ReceiveQueue * q);
nfile_ReceiveQueue * nfile_ReceiveQueue_renew(nfile_ReceiveQueue * q);

//--------------------------------------------------------------------------------------------

typedef struct nfile_SendState_t
{
  bool_t        initialized;

  // the file queue
  nfile_SendQueue queue;

  // other data
  Uint32 crc_seed;

  // thread info
  NetThread nthread;

} nfile_SendState;

static nfile_SendState * nfile_SendState_create();
static bool_t            nfile_SendState_destroy(nfile_SendState ** snd);

static nfile_SendState * nfile_SendState_new(nfile_SendState * snd);
static bool_t            nfile_SendState_delete(nfile_SendState * snd);
static retval_t          nfile_SendState_initialize(nfile_SendState * snd);
static retval_t          nfile_SendState_startUp(nfile_SendState * snd);
static retval_t          nfile_SendState_shutDown(nfile_SendState * snd);
static retval_t          nfile_SendState_startThread(nfile_SendState * snd);
static retval_t          nfile_SendState_stopThread(nfile_SendState * snd);

static retval_t          _nfile_Receive_initialize();
//--------------------------------------------------------------------------------------------

typedef struct nfile_ReceiveState_t
{
  bool_t     initialized;

  // the file queue
  nfile_ReceiveQueue queue;

  // thread info
  NetThread nthread;

} nfile_ReceiveState;

static nfile_ReceiveState * nfile_ReceiveState_create();
static bool_t               nfile_ReceiveState_destroy(nfile_ReceiveState ** rec);

static nfile_ReceiveState * nfile_ReceiveState_new(nfile_ReceiveState * rec);
static bool_t               nfile_ReceiveState_delete(nfile_ReceiveState * rec);
static retval_t             nfile_ReceiveState_initialize(nfile_ReceiveState * rec);
static retval_t             nfile_ReceiveState_startUp(nfile_ReceiveState * rec);
static retval_t             nfile_ReceiveState_shutDown(nfile_ReceiveState * rec);
static retval_t             nfile_ReceiveState_startThread(nfile_ReceiveState * rec);
static retval_t             nfile_ReceiveState_stopThread(nfile_ReceiveState * rec);

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

//retval_t nfile_startUp(NFileState * nfs, NetState * ns)
//{
//  if(NULL == nfs) return rv_error;
//
//  if(nfile_initialized) return rv_succeed;
//
//  NFileState_new(nfs);
//  nfile_initialized = (rv_succeed == NFileState_initialize(nfs, ns, &nfile_SendState, &nfile_ReceiveState));
//
//  if(!nfile_atexit_registered)
//  {
//    atexit(nfile_Quit);
//    nfile_atexit_registered = btrue;
//  }
//
//  return nfile_initialized ? rv_succeed : rv_fail;
//}

//--------------------------------------------------------------------------------------------
//retval_t nfile_shutDown()
//{
//  if(NULL == nfs) return rv_error;
//  if(!nfile_initialized) return rv_succeed;
//
//  // shut down all services
//  NetHost_shutDown( sv_getHost() );
//  NetHost_shutDown( cl_getHost() );
//  NetHost_shutDown( nfile_getHost() );
//
//  return rv_succeed;
//}

//--------------------------------------------------------------------------------------------
//void nfile_Quit(void)
//{
//  if(nfile_initialized)
//  {
//    NFileState_delete(&ANFileState);
//  }
//}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
NFileState * NFileState_create(NetState * ns)
{
  NFileState * nfs = (NFileState *)calloc(1, sizeof(NFileState));
  return NFileState_new(nfs, ns);
}

//--------------------------------------------------------------------------------------------
bool_t NFileState_destroy(NFileState ** pnfs)
{
  bool_t retval;

  if(NULL == pnfs || NULL == *pnfs) return bfalse;
  if( !(*pnfs)->initialized ) return btrue;

  retval = NFileState_delete(*pnfs);
  FREE( *pnfs );

  return retval;
}

//--------------------------------------------------------------------------------------------
NFileState * NFileState_new(NFileState * nfs, NetState * ns)
{
  fprintf( stdout, "NFileState_new()\n");

  if(NULL == nfs || nfs->initialized) return nfs;

  if(nfs->initialized) NFileState_delete(nfs);

  memset( nfs, 0, sizeof(NFileState) );

  if(net_Started())
  {
    nfs->host     = nfile_getHost();
    nfs->net_guid = net_addService(nfs->host, nfs);
    nfs->net_guid = NetHost_register(nfs->host, net_handlePacket, nfs, nfs->net_guid);
  };

  nfs->parent = ns;

  nfs->initialized = btrue;

  return nfs;
}

//--------------------------------------------------------------------------------------------
bool_t NFileState_delete(NFileState * nfs)
{
  if(NULL == nfs) return bfalse;

  if(!nfs->initialized) return btrue;

  net_removeService(nfs->net_guid);
  NetHost_unregister(nfs->host, nfs->net_guid);

  nfile_SendState_delete( nfs->snd );
  nfile_ReceiveState_delete( nfs->rec );

  nfs->initialized = bfalse;

  return btrue;
}


//--------------------------------------------------------------------------------------------
retval_t NFileState_initialize(NFileState * nfs)
{
  NetHost           * nh;
  nfile_SendState    * loc_snd;
  nfile_ReceiveState * loc_rec;

  if(NULL == nfs) return rv_error;
  if(nfs->initialized) return rv_succeed;

  //---- initialize the net file transfer host
  nh = nfile_getHost();
  NetHost_startUp(nh, NET_EGOBOO_NETWORK_PORT);

  // ensure that the send state is initialized
  loc_snd = _nfile_getSend();

  // ensure that the receive state is initialized
  loc_rec = _nfile_getReceive();

  return rv_succeed;
}

//--------------------------------------------------------------------------------------------
retval_t NFileState_startUp(NFileState * nfs)
{
  // BB > Start the NFileState. 
  //      If it has not been initialized, initialize it.
  //      If it was initialized, restart everything.

  if(NULL == nfs || !nfs->initialized) return rv_error;

  // if the network is not on, don't bother
  if (!net_Started())
  {
    NFileState_shutDown(nfs);
    return rv_error; 
  }

  // make sure that the host is actually Active
  if( !nfile_Started() )
  {
    retval_t rv = _nfile_startUp();
    if(rv_succeed != rv) return rv;
  }

  // ensure that the worker threads are started
  nfile_SendState_startUp(nfs->snd);
  nfile_ReceiveState_startUp(nfs->rec);

  return nfs->host->nthread.Active ? rv_succeed : rv_fail;
}

//--------------------------------------------------------------------------------------------
retval_t NFileState_shutDown(NFileState * nfs)
{
  // BB > Shut or pause the NFileState. 

  if(NULL == nfs || !nfs->initialized) return rv_error;

  net_logf("NET INFO: NFileState_shutDown() - Shutting down network file server... ");

  nfile_SendState_shutDown(nfs->snd);
  nfile_ReceiveState_shutDown(nfs->rec);

  //TODO : the host will have to have to count the number of locks on it
  //     : and shut down only when the number of locks goes to 0
  //NetHost_shutDown( nfs->host );

  net_logf("Done\n");

  return rv_succeed;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
nfile_SendState * nfile_SendState_create()
{
  nfile_SendState * snd = (nfile_SendState *)calloc(1, sizeof(nfile_SendState));
  return nfile_SendState_new( snd );
}

//--------------------------------------------------------------------------------------------
bool_t nfile_SendState_destroy(nfile_SendState ** snd)
{
  bool_t retval;

  if(NULL == snd || NULL == (*snd) ) return bfalse;
  if( !(*snd)->initialized) return btrue;

  retval = nfile_SendState_delete(*snd);

  FREE( *snd );

  return retval;
}

//--------------------------------------------------------------------------------------------
nfile_SendState * nfile_SendState_new(nfile_SendState * snd)
{
  fprintf( stdout, "nfile_SendState_new()\n");

  if(NULL == snd || snd->initialized) return snd;

  memset(snd, 0, sizeof(nfile_SendState));

  NetThread_new( &(snd->nthread), _nfile_sendCallback);
  nfile_SendQueue_new( &(snd->queue) );

  snd->initialized = btrue;

  return snd;
}

//--------------------------------------------------------------------------------------------
bool_t nfile_SendState_delete(nfile_SendState * snd)
{
  if(NULL == snd) return bfalse;
  
  if(!snd->initialized) return btrue;

  // terminate the thread (not nicely)
  SDL_KillThread(snd->nthread.Thread);

  // delete (and auto-deallocate) the queue
  nfile_SendQueue_delete( &(snd->queue) );

  snd->initialized = bfalse;

  return btrue;
}

//--------------------------------------------------------------------------------------------
retval_t nfile_SendState_initialize(nfile_SendState * snd)
{
  // BB> initializes an instance of nfile_SendState

  if(NULL == snd) return rv_error;

  if(!snd->initialized)
  {
    nfile_SendState_new( snd );
  };

  // set the seed value for CRC calculations
  snd->crc_seed = (Uint32) time( NULL );

  // set the the existing thread (if any) to pause
  snd->nthread.KillMe  = bfalse;
  snd->nthread.Paused = btrue;

  return rv_succeed;
}

//--------------------------------------------------------------------------------------------
retval_t nfile_SendState_startUp(nfile_SendState * snd)
{
  NetHost * nh;

  if(NULL == snd) return rv_error;

  nh = nfile_getHost();

  if(!snd->initialized)
  {
    nfile_SendState_new(snd);
    nfile_SendState_initialize(snd);
  }

  // set the thread to run
  snd->nthread.Paused = bfalse;
  snd->nthread.KillMe = bfalse;

  // if the thread doesn't exist, create it
  if(NULL == snd->nthread.Thread)
  {
    snd->nthread.Thread = SDL_CreateThread(_nfile_sendCallback, snd);
  }
  if(NULL == snd->nthread.Thread)
  {
    snd->nthread.Paused = btrue;
    net_logf("NET ERROR: nfile_SendState_startUp() - Error starting _nfile_sendCallback() thread.\n");
    return rv_error;
  }

  return rv_succeed;
}

//--------------------------------------------------------------------------------------------
retval_t nfile_SendState_shutDown(nfile_SendState * snd)
{
  if(NULL == snd || !snd->initialized) return rv_error;

  snd->nthread.Paused = btrue;

  return rv_succeed;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
nfile_ReceiveState * nfile_ReceiveState_create()
{
  nfile_ReceiveState * rec = (nfile_ReceiveState *)calloc(1, sizeof(nfile_ReceiveState));
  return nfile_ReceiveState_new( rec);
}

//--------------------------------------------------------------------------------------------
bool_t nfile_ReceiveState_destroy(nfile_ReceiveState ** rec)
{
  bool_t retval;

  if(NULL == rec || NULL == (*rec) ) return bfalse;
  if( !(*rec)->initialized) return btrue;

  retval = nfile_ReceiveState_delete(*rec);

  FREE( *rec );

  return retval;
}

//--------------------------------------------------------------------------------------------
nfile_ReceiveState * nfile_ReceiveState_new(nfile_ReceiveState * rec)
{
  fprintf( stdout, "nfile_ReceiveState_new()\n");

  if(NULL == rec || rec->initialized) return rec;

  memset(rec, 0, sizeof(nfile_ReceiveState));

  NetThread_new( &(rec->nthread), _nfile_receiveCallback);
  nfile_ReceiveQueue_new( &(rec->queue) );

  rec->initialized = btrue;

  return rec;
}

//--------------------------------------------------------------------------------------------
bool_t nfile_ReceiveState_delete(nfile_ReceiveState * rec)
{
  if(NULL == rec) return bfalse;
  
  if(!rec->initialized) return btrue;

  // terminate the thread (not nicely)
  SDL_KillThread(rec->nthread.Thread);

  // delete (and auto-deallocate) the queue
  nfile_ReceiveQueue_delete( &(rec->queue) );

  rec->initialized = bfalse;

  return btrue;
}

//--------------------------------------------------------------------------------------------
retval_t nfile_ReceiveState_initialize(nfile_ReceiveState * rec)
{
  // BB> initializes an instance of nfile_ReceiveState

  if(NULL == rec) return rv_error;

  if(!rec->initialized)
  {
    nfile_ReceiveState_new( rec );
  };

  // set the the existing thread (if any) to pause
  rec->nthread.KillMe = bfalse;
  rec->nthread.Paused = btrue;

  return rv_succeed;
}

//--------------------------------------------------------------------------------------------
retval_t nfile_ReceiveState_startUp(nfile_ReceiveState * rec)
{
  if(NULL == rec) return rv_error;

  if(!rec->initialized)
  {
    nfile_ReceiveState_new(rec);
    nfile_ReceiveState_initialize(rec);
  }

  // set the thread to run
  rec->nthread.Paused = bfalse;
  rec->nthread.KillMe = bfalse;

  // if the thread doesn't exist, create it
  if(NULL == rec->nthread.Thread)
  {
    rec->nthread.Thread = SDL_CreateThread(_nfile_receiveCallback, rec);
  }
  if(NULL == rec->nthread.Thread)
  {
    rec->nthread.Paused = btrue;
    net_logf("NET ERROR: nfile_ReceiveState_startUp() - Error starting _nfile_receiveCallback() thread.\n");
    return rv_error;
  }

  return rv_succeed;
}

//--------------------------------------------------------------------------------------------
retval_t nfile_ReceiveState_shutDown(nfile_ReceiveState * rec)
{
  if(NULL == rec || !rec->initialized) return rv_error;

  rec->nthread.Paused = btrue;

  return rv_succeed;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
bool_t nfile_ReceiveInfo_alloc( nfile_ReceiveInfo * nri, size_t size)
{
  bool_t retval = bfalse;

  if(NULL == nri) return retval;

  if(nri->allocated)
  {
    nri->buffer      = (Uint8 *)realloc(nri->buffer, size*sizeof(Uint8));
    nri->buffer_size = nri->buffer_size;
    retval = btrue;
  }
  else
  {
    nri->buffer = (Uint8*)calloc(size, sizeof(Uint8));
    retval = btrue;
  }

  if(NULL == nri->buffer)
  {
    retval = bfalse;
    nri->allocated   = bfalse;
    nri->buffer_size = 0;
  };

  return retval;
}

bool_t nfile_ReceiveInfo_dealloc( nfile_ReceiveInfo * nri)
{
  if(NULL == nri) return bfalse;
  if(!nri->allocated) return btrue;

  FREE(nri->buffer);
  nri->buffer_size = 0;
  nri->allocated   = bfalse;

  return btrue;
}

retval_t nfile_ReceiveQueue_add(NFileState * nfs, ENetEvent * event, char * dest)
{
  int temp_tail;
  nfile_ReceiveQueue * queue = NULL;
  nfile_ReceiveInfo  * state = NULL;

  if(NULL == nfs || NULL == event)
  {
    net_logf("NET ERROR: nfile_ReceiveQueue_add() - Called with invalid parameters.\n");
    return rv_error;
  }

  if(NULL == dest || '\0' == dest[0])
  {
    net_logf("NET ERROR: nfile_ReceiveQueue_add() - null destination file string.\n");
    return rv_error;
  }

  queue = &(nfs->rec->queue);
  if( NULL == queue || !queue->initialized )
  {
    net_logf("NET ERROR: nfile_SendQueue_add() - Invalid queue.\n");
    return rv_error;
  }

  temp_tail = (queue->TransferTail + 1) % NET_MAX_FILE_TRANSFERS;

  if(temp_tail == queue->TransferHead)
  {
    net_logf("NET ERROR: nfile_ReceiveQueue_add(): Warning!  Queue tail caught up with the head!\n");
    return rv_fail;
  }

  net_logf("--------------------------------------------------\n");
  net_logf("NET INFO: nfile_ReceiveQueue_add() - Adding file transfer.\n");

  // TransferTail should already be pointed at an open slot in the queue.
  state = queue->transferStates + queue->TransferTail;
  assert(state->destName[0] == '\0');

  // initialize the state
  nfile_ReceiveInfo_alloc(state, event->packet->dataLength);
  state->sender = event->peer;
  state->event_type = event->type;
  memcpy(state->buffer, event->packet->data, event->packet->dataLength);
  strncpy(state->destName, dest, NET_MAX_FILE_NAME);

  // advance the tail index
  queue->numTransfers++;
  queue->TransferTail++;
  queue->TransferTail = (queue->TransferTail + 1) % NET_MAX_FILE_TRANSFERS;

  return rv_succeed;
}

//--------------------------------------------------------------------------------------------
//bool_t nfile_dispatchPackets(NetHost * nh)
//{
//  ENetEvent event;
//  CListIn_Info cin_info, *pcin_info;
//  char hostName[64] = { '\0' };
//  NetRequest * prequest;
//  size_t copy_size;
//
//  if(NULL==nh->Host) return bfalse;
//
//  while (enet_host_service(nh->Host, &event, 0) > 0)
//  {
//    net_logf("--------------------------------------------------\n");
//    net_logf("NET INFO: nfile_dispatchPackets() - Received event... ");
//    if(!nh->nthread.Active) 
//    {
//      net_logf("Ignored\n");
//      continue;
//    }
//    net_logf("Processing... ");
//
//    switch (event.type)
//    {
//    case ENET_EVENT_TYPE_RECEIVE:
//      net_logf("ENET_EVENT_TYPE_RECEIVE\n");
//
//      prequest = NetRequest_check(nh->asynch, NETHOST_REQ_COUNT, &event);
//      if( NULL != prequest && prequest->waiting )
//      {
//        // we have received a packet that someone is waiting for
//
//        net_logf("NET INFO: nfile_dispatchPackets() - Received requested packet \"%s\" from %s:%04x\n", net_crack_enet_packet_type(event.packet), convert_host(event.peer->address.host), event.peer->address.port);
//
//        // copy the data from the packet to a waiting buffer
//        copy_size = MIN(event.packet->dataLength, prequest->data_size);
//        memcpy(prequest->data, event.packet->data, copy_size);
//        prequest->data_size = copy_size;
//
//        // tell the other thread that we're done
//        prequest->received = btrue;
//      }
//      else if(!net_handlePacket(nfs->parent, &event))
//      {
//        net_logf("NET WARNING: nfile_dispatchPackets() - Unhandled packet\n");
//      }
//
//      enet_packet_destroy(event.packet);
//      break;
//
//    case ENET_EVENT_TYPE_CONNECT:
//      net_logf("ENET_EVENT_TYPE_CONNECT\n");
//
//      cin_info.Address = event.peer->address;
//      cin_info.Hostname[0] = '\0';
//      cin_info.Name[0] = '\0';
//
//      // Look up the hostname the player is connecting from
//      //enet_address_get_host(&event.peer->address, hostName, 64);
//      //strncpy(nh->Hostname[cnt], hostName, sizeof(nh->Hostname[cnt]));
//
//      // see if we can add the 'player' to the connection list
//      pcin_info = CListIn_add( &(nh->cin), &cin_info );
//
//      if(NULL == pcin_info)
//      {
//        net_logf("NET WARN: nfile_dispatchPackets() - Too many connections. Refusing connection from \"%s\" (%s:%04x)\n", hostName, convert_host(event.peer->address.host), event.peer->address.port);
//        net_stopPeer(event.peer);
//      }
//
//      // link the player info to the event.peer->data field
//      event.peer->data = &(pcin_info->Slot);
// 
//      break;
//
//    case ENET_EVENT_TYPE_DISCONNECT:
//      net_logf("ENET_EVENT_TYPE_DISCONNECT\n");
//
//      if( !CListIn_remove( &(nh->cin), event.peer ) )
//      {
//        net_logf("NET INFO: nfile_dispatchPackets() - Unknown connection closing from \"%s\" (%s:%04x)\n", hostName, convert_host(event.peer->address.host), event.peer->address.port);
//      }
//      break;
//
//    case ENET_EVENT_TYPE_NONE:
//      net_logf("ENET_EVENT_TYPE_NONE\n");
//      break;
//
//    default:
//      net_logf("UNKNOWN\n");
//      break;
//
//    }
//  }
//
//  // !!make sure that all of the packets get sent!!
//  if(NULL != nh->Host)
//  {
//    enet_host_flush(nh->Host);
//  }
//
//  return btrue;
//};
//
//

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
int  net_pendingFileTransfers(nfile_SendQueue * queue)
{
  if(NULL == queue || !queue->initialized) return 0;

  return queue->numTransfers;
}

//------------------------------------------------------------------------------
static retval_t net_doFileTransfer(nfile_SendInfo *state)
{
  ENetPacket *packet;
  size_t nameLen, fileSize;
  Uint32 networkSize;
  FILE *file;
  char *p;
  retval_t retval;

  Uint8 *buffer;
  size_t buffer_size;

  file = fopen(state->sourceName, "rb");
  if (file)
  {
    net_logf("NET INFO: net_doFileTransfer() - Attempting to send %s to %s\n", state->sourceName, state->destName);

    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Make room for the file's name
    nameLen = strlen(state->destName) + 1;
    buffer_size = nameLen;

    // And for the file's size
    buffer_size += 6; // Uint32 size, and Uint16 message type
    buffer_size += fileSize;

    buffer = (Uint8*)calloc(buffer_size, sizeof(Uint8));
    *(Uint16*)buffer = ENET_HOST_TO_NET_16(NET_TRANSFER_FILE);

    // Add the string and file length to the buffer
    p = buffer + 2;
    strcpy(p, state->destName);
    p += nameLen;

    networkSize = ENET_HOST_TO_NET_32((Uint32)fileSize);
    *(size_t*)p = networkSize;
    p += 4;

    fread(p, 1, fileSize, file);
    fclose(file);

    packet = enet_packet_create(buffer, buffer_size, ENET_PACKET_FLAG_RELIABLE);
    if(NULL != packet)
    {
      if(NULL == state->target)
      {
        // a NULL in this field means send to all peers
        net_logf("NET INFO: net_doFileTransfer() - Broadcasting NET_TRANSFER_FILE... ");
        enet_host_broadcast(state->host, NET_GUARANTEED_CHANNEL, packet);
        net_logf("Done\n");
      }
      else
      {
        net_logf("NET INFO: net_doFileTransfer() - Sendind NET_TRANSFER_FILE... ");
        enet_peer_send(state->target, NET_GUARANTEED_CHANNEL, packet);
        net_logf("Done\n");
      }
    }
    else
    {
      net_logf("NET ERROR: net_doFileTransfer() - Unable to generate packet.\n");
    }

    FREE(buffer);
    buffer_size = 0;

    retval = rv_succeed;
  }
  else
  {
    retval = rv_error;
    net_logf("NET WARNING: net_doFileTransfer() - Could not open file %s to send it!\n", state->sourceName);
  }

  return retval;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
retval_t nfhost_startThreads()
{
  // BB > start the main packet hosting thread and
  //      start the worker threads

  NetHost * nh;
  retval_t retval, snd_return, rec_return;

  // fail to start
  nh = nfile_getHost();
  if(NULL == nh)
  {
    return rv_error;
  }

  if(NULL == nh->nthread.Thread)
  {
    // thread has never been started
    nh->nthread.Thread = SDL_CreateThread(nfhost_HostCallback, nh);
  }
  nh->nthread.Paused = bfalse;
  nh->nthread.KillMe  = bfalse;
  retval = rv_succeed;

  // start the worker threads
  snd_return = nfile_SendState_startThread( _nfile_getSend() );
  rec_return = nfile_ReceiveState_startThread( _nfile_getReceive() );

  // try to give back a reasonable error code if an error occurred
  if(rv_fail == snd_return || rv_fail == rec_return)
  {
    retval = rv_fail;
  }

  if(rv_error == snd_return || rv_error == rec_return)
  {
    retval = rv_error;
  }

  return retval;
}

//------------------------------------------------------------------------------
retval_t nfhost_stopThreads(NFileState * nfs)
{
  // BB > do cleanup when exiting the "net file" thread

  // bad parameters
  if(NULL == nfs || !nfs->initialized)
  {
    net_logf("NET ERROR: nfhost_stopThreads() - failed.\n");
    return rv_error;
  }

  // tell the worker threads to stop, too
  nfs->snd->nthread.KillMe = btrue;
  nfs->rec->nthread.KillMe = btrue;

  // close all connections to the host
  NetHost_close( nfs->host, nfs );

  // set the thread status in the "killed" state
  nfs->host->nthread.KillMe  = btrue;
  nfs->host->nthread.Paused = btrue;

  return rv_succeed;
}

//--------------------------------------------------------------------------------------------
int nfhost_HostCallback(void * data)
{
  NetHost   * nf_host;
  NetThread * nthread;
  retval_t   dispatch_return;

  retval_t retval;

  nf_host = (NetHost *)data;

  // try to start
  net_logf("NET INFO: nfhost_HostCallback() thread - starting... ");
  retval = nfhost_startThreads();
  if(rv_error == retval)
  {
    net_logf("Error!\n");
    return retval;
  }
  else if(rv_fail == retval)
  {
    net_logf("Failed!\n");
    return retval;
  }
  net_logf("Success!\n");

  nthread = &(nf_host->nthread);
  while(NULL!=nf_host && !nthread->KillMe)
  {
    nf_host = nfile_getHost();
    nthread = &(nf_host->nthread);

    if(!nthread->Paused)
    {
      dispatch_return = NetHost_dispatch(nf_host) ? rv_succeed : rv_fail;
    }

    // Let the OS breathe
    SDL_Delay(20);
  }

  net_logf("NET INFO: nfhost_HostCallback() thread - terminating... ");
  retval = nfhost_stopThreads();
  if(rv_error == retval)
  {
    net_logf("Error!\n");
  }
  else if(rv_fail == retval)
  {
    net_logf("Fail!\n");
  }
  else
  {
    net_logf("Success!\n");
  }

  return retval;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

retval_t nfile_SendState_startThread(nfile_SendState * snd)
{
  // BB > start the "send file" worker thread

  bool_t   thread_valid;

  // fail to start
  if(NULL == snd)
  {
    return rv_error;
  }

  // do whatever it takes to get the thread running again
  if(NULL == snd->nthread.Thread)
  {
    // thread has never been started
    snd->nthread.Thread = SDL_CreateThread(_nfile_sendCallback, snd);
  }
  thread_valid = (NULL != snd->nthread.Thread);

  snd->nthread.Paused = !thread_valid;
  snd->nthread.KillMe  = !thread_valid;
  
  return thread_valid ? rv_succeed : rv_fail;
}

//------------------------------------------------------------------------------
retval_t nfile_SendState_stopThread(nfile_SendState * snd)
{
  // BB > do cleanup when exiting a "send file" worker thread

  // bad parameters
  if(NULL == snd || !snd->initialized)
  {
    net_logf("NET ERROR: nfile_SendState_stopThread() - failed.\n");
    return rv_error;
  }
  
  // flush the queue of any unfinished files
  nfile_SendQueue_renew( &(snd->queue) );

  // set the thread status in the "killed" state
  snd->nthread.KillMe  = btrue;
  snd->nthread.Paused = btrue;

  return rv_succeed;
}


//------------------------------------------------------------------------------
retval_t nfile_ReceiveState_startThread(nfile_ReceiveState * rec)
{
  // BB > start the "send file" worker thread

  bool_t   thread_valid;

  // fail to start
  if(NULL == rec)
  {
    return rv_error;
  }

  // do whatever it takes to get the thread running again
  if(NULL == rec->nthread.Thread)
  {
    // thread has never been started
    rec->nthread.Thread = SDL_CreateThread(_nfile_receiveCallback, rec);
  }
  thread_valid = (NULL != rec->nthread.Thread);

  rec->nthread.Paused = !thread_valid;
  rec->nthread.KillMe  = !thread_valid;
  
  return thread_valid ? rv_succeed : rv_fail;
}

//------------------------------------------------------------------------------
retval_t nfile_ReceiveState_stopThread(nfile_ReceiveState * rec)
{
  // BB > do cleanup when exiting a "receive file" worker thread

  // bad parameters
  if(NULL == rec || !rec->initialized)
  {
    net_logf("NET ERROR: nfile_ReceiveState_stopThread() - failed.\n");
    return rv_error;
  }
  
  // flush the queue of any unfinished files
  nfile_ReceiveQueue_renew( &(rec->queue) );

  // set the thread status in the "killed" state
  rec->nthread.KillMe  = btrue;
  rec->nthread.Paused = btrue;

  return rv_succeed;
}

//------------------------------------------------------------------------------
int _nfile_sendCallback(void * data)
{
  NetHost        * nh;
  nfile_SendState * sfs;
  nfile_SendQueue * queue;
  nfile_SendInfo *state;
  SYS_PACKET egopkt;
  int rand_idx;
  Uint32 CRC;
  retval_t retval, wait_return;
  bool_t advance_head = bfalse;

  sfs = (nfile_SendState *)data;

  // try to start
  net_logf("NET INFO: _nfile_sendCallback() thread - starting... ");
  retval = nfile_SendState_startThread(sfs);   
  if(rv_error == retval)
  {
    net_logf("Error!\n");
    return retval;
  }
  else if(rv_fail == retval)
  {
    net_logf("Failed!\n");
    return retval;
  }
  net_logf("Success!\n");

  queue = &(sfs->queue);
  if(NULL == queue || !queue->initialized)
  {
    net_logf("NET ERROR: _nfile_sendCallback() - Thread terminated because of invalid data.\n");

    return rv_fail;
  }

  net_logf("NET INFO: _nfile_sendCallback() thread - starting normally.\n");

  while(!sfs->nthread.KillMe)
  {
    // return if there is nothing to do
    if(queue->numTransfers <= 0) { SDL_Delay(200); continue; }

    // grab the host
    nh = nfile_getHost();
    if(NULL == nh) { SDL_Delay(200); continue; }

    advance_head = bfalse;
    state = queue->transferStates + queue->TransferHead;

    net_logf("NET INFO: _nfile_sendCallback() - processing \"%s\"\n", state->sourceName);


    // Check and see if this is a directory, instead of a file
    if (fs_fileIsDirectory(state->sourceName))
    {
      // grab the host
      NetHost * nh  = nfile_getHost();

      // Tell the target to create a directory
      net_logf("NET INFO: net_updateFileTranfers: Creating directory %s on target... ", state->destName);
      net_startNewSysPacket(&egopkt);
      sys_packet_addUint16(&egopkt, NET_CREATE_DIRECTORY);
      sys_packet_addString(&egopkt, state->destName);
      net_sendSysPacketToPeerGuaranteed(state->target, &egopkt);
      net_logf("Sent\n");

      net_logf("NET INFO: _nfile_sendCallback() - Waiting for confirmation... ");
      wait_return = net_waitForPacket(nh->asynch, state->target, 5000, NET_TRANSFER_ACK, NULL);
      if(rv_fail == wait_return || rv_error == wait_return) 
      {
        net_logf("Timed out!\n");

        // Do not advance the queue->TransferHead. Try to send the file again
        advance_head = bfalse;
      }
      else
      {
        // advance the queue->TransferHead
        net_logf("Confirmed.\n");

        // Advance the queue->TransferHead
        advance_head = bfalse;
      } 

    }
    else
    {
      // make sure that the file needs to be transferred
      rand_idx = sfs->crc_seed;
      util_calculateCRC(state->sourceName, rand_idx, &CRC);
      retval = nfhost_checkCRC(state->target, state->destName, rand_idx, CRC);

      if(rv_fail != retval)
      {
        // File does not need to be transferred
        advance_head = btrue;
      }
      else
      {
        // CRCs are different. Transfer the files
        if(rv_succeed != net_doFileTransfer(state))
        {
          // File transfer just failed.  File does not exist, Bad destination, etc.
          net_logf("NET WARN: _nfile_sendCallback() - File transfer failed.\n");

          // advance the queue->TransferHead
          advance_head = btrue;
        }
        else
        {
          // file is sent
          net_logf("NET INFO: _nfile_sendCallback() - File sent. Waiting for confirmation... ");
          wait_return = net_waitForPacket(nh->asynch, state->target, 5000, NET_TRANSFER_ACK, NULL);
          if(rv_fail == wait_return || rv_error == wait_return) 
          {
            net_logf("Timed out!\n");

            // Do not advance the queue->TransferHead. Try to send the file again
            advance_head = bfalse;
          }
          else
          {
            // advance the queue->TransferHead
            net_logf("Confirmed.\n");

            // Advance the queue->TransferHead
            advance_head = bfalse;
          } 
        }
      }

      if(advance_head)
      {
        // update transfer queue state
        memset(state, 0, sizeof(nfile_SendInfo));
        queue->TransferHead++;
        queue->numTransfers--;
        if (queue->TransferHead >= NET_MAX_FILE_TRANSFERS)
        {
          queue->TransferHead = 0;
        }
      }
    }

    // Let the OS breathe
    SDL_Delay(20);
  }

  net_logf("NET INFO: _nfile_sendCallback() thread - terminating... ");
  retval = nfile_SendState_stopThread(sfs);
  if(rv_error == retval)
  {
    net_logf("Error!\n");
  }
  else if(rv_fail == retval)
  {
    net_logf("Fail!\n");
  }
  else
  {
    net_logf("Success!\n");
  }

  return retval;
}

//------------------------------------------------------------------------------
int _nfile_receiveCallback(void * data)
{
  nfile_ReceiveState * rfs;
  nfile_ReceiveQueue * queue;
  nfile_ReceiveInfo  * state;
  FILE *file;
  SYS_PACKET egopkt;
  bool_t succeed;
  bool_t handled;
  retval_t retval;

  rfs = (nfile_ReceiveState *)data;

  // try to start
  net_logf("NET INFO: _nfile_receiveCallback() thread - starting... ");
  retval = nfile_ReceiveState_startThread(rfs);
  if(rv_error == retval)
  {
    net_logf("Error!\n");
    return retval;
  }
  else if(rv_fail == retval)
  {
    net_logf("Failed!\n");
    return retval;
  }
  net_logf("Success!\n");

  queue = &(rfs->queue);
  if(NULL == queue || !queue->initialized)
  {
    net_logf("NET ERROR: _nfile_receiveCallback() - Thread terminated because of invalid data.\n");

    return rv_fail;
  }

  net_logf("NET INFO: _nfile_receiveCallback() thread - starting normally.\n");

  while(!rfs->nthread.KillMe)
  {
    // return if there is nothing to do
    if(queue->numTransfers <= 0) { SDL_Delay(200); continue; }

    state   = queue->transferStates + queue->TransferHead;
    succeed = bfalse;

    net_logf("NET INFO: _nfile_receiveCallback() - processing \"%s\"\n", state->destName);

    switch(state->event_type)
    {
      case NET_CREATE_DIRECTORY:
        
        net_logf("NET INFO: _nfile_receiveCallback() - NET_CREATE_DIRECTORY \"%s\"... ", state->destName);

        // convert the slashes in the directory name something valid on this system
        str_convert_sys(state->destName, NET_MAX_FILE_NAME);

        if(fs_fileIsDirectory(state->destName))
        {
          // Try to create the directory
          succeed = (0 != fs_createDirectory( state->destName ));
        }
        
        if(succeed)
        {
          net_logf("Succeeded!");
        }
        else
        {
          net_logf("Failed! Bad directory name?");
        }
        break;

      case NET_TRANSFER_FILE:
        succeed = bfalse;

        file = fopen(state->destName, "wb");
        if(NULL != file)
        {
          fwrite(state->buffer, 1, state->buffer_size, file);
          succeed = (0 != ferror(file));
          fclose(file);
        }

        handled = btrue;
        break;

      default:
        handled = bfalse;
        break;
    }

    if(handled)
    {
      // acknowledge the packet
      net_startNewSysPacket(&egopkt);
      sys_packet_addUint16(&egopkt, NET_TRANSFER_ACK);
      sys_packet_addUint8(&egopkt, succeed);
      net_sendSysPacketToPeerGuaranteed(state->sender, &egopkt);
    }

    // update transfer queue state
    nfile_ReceiveInfo_dealloc(state);

    queue->TransferHead++;
    queue->numTransfers--;
    if (queue->TransferHead >= NET_MAX_FILE_TRANSFERS)
    {
      queue->TransferHead = 0;
    }

    // Let the OS breathe
    SDL_Delay(20);
  }

  net_logf("NET INFO: _nfile_receiveCallback() thread - terminating... ");
  retval = nfile_ReceiveState_stopThread(rfs);
  if(rv_error == retval)
  {
    net_logf("Error!\n");
  }
  else if(rv_fail == retval)
  {
    net_logf("Fail!\n");
  }
  else
  {
    net_logf("Success!\n");
  }

  return retval;
}



//------------------------------------------------------------------------------
retval_t nfile_SendQueue_add(NFileState * nfs, ENetAddress * target_address, char *source, char *dest)
{
  int temp_tail;
  nfile_SendQueue * queue = NULL;
  nfile_SendInfo  * state = NULL;

  if( NULL == nfs || NULL == nfs->host->Host )
  {
    net_logf("NET ERROR: nfile_SendQueue_add() - Called with invalid parameters.\n");
    return rv_error;
  }

  if(NULL == source || '\0' == source[0])
  {
    net_logf("NET ERROR: nfile_SendQueue_add() - null source file.\n");
    return rv_error;
  }

  if(NULL == dest || '\0' == dest[0])
  {
    net_logf("NET ERROR: nfile_SendQueue_add() - null dest file.\n");
    return rv_error;
  }

  queue = &(nfs->snd->queue);
  if( NULL == queue || !queue->initialized )
  {
    net_logf("NET ERROR: nfile_SendQueue_add() - Invalid queue.\n");
    return rv_error;
  }

  temp_tail = (queue->TransferTail + 1) % NET_MAX_FILE_TRANSFERS;

  if(temp_tail == queue->TransferHead)
  {
    net_logf("NET ERROR: net_copyFileToAllPeers: Warning!  Queue tail caught up with the head!\n");
    return rv_fail;
  }

  net_logf("--------------------------------------------------\n");
  net_logf("NET INFO: nfile_SendQueue_add() - Adding file transfer from %s:%04x:\"%s\" to %s:%04x:\"%s\".\n", 
    convert_host(nfs->host->Host->address.host), nfs->host->Host->address.port, source,
    convert_host(target_address->host), target_address->port, dest);

  // TransferTail should already be pointed at an open slot in the queue.
  state = queue->transferStates + queue->TransferTail;
  assert(state->sourceName[0] == 0);

  state->host   = nfs->host->Host; 
  state->target = enet_host_connect(state->host, target_address, NET_EGOBOO_NUM_CHANNELS);
  strncpy(state->sourceName, source, NET_MAX_FILE_NAME);
  strncpy(state->destName,   dest,   NET_MAX_FILE_NAME);

  // advance the tail index
  queue->numTransfers++;
  queue->TransferTail++;
  queue->TransferTail = (queue->TransferTail + 1) % NET_MAX_FILE_TRANSFERS;

  return rv_succeed;
}



//--------------------------------------------------------------------------------------------
retval_t nfhost_checkCRC(ENetPeer * peer, const char * source, Uint32 seed, Uint32 CRC)
{
  NetHost * nh;
  Uint8  buffer[NET_REQ_SIZE];
  SYS_PACKET egopkt;
  Uint32* usp, temp_CRC = 0;
  retval_t wait_return;

  if(NULL == peer)
  {
    net_logf("NET ERROR: nfhost_checkCRC() - Called with invalid parameters.\n");
    return rv_error;
  }

  if(NULL == source || '\0' == source[0])
  {
    net_logf("NET ERROR: nfhost_checkCRC() - null source file.\n");
    return rv_error;
  }

  nh = nfile_getHost();
  if( NULL == nh )
  {
    net_logf("NET ERROR: nfhost_checkCRC() - host cannot be started.\n");
    return rv_error;
  }

  // request a CRC calculation from the peer for the file source
  net_logf("NET INFO: nfhost_checkCRC() - Sending CRC request to target.\n");
  net_startNewSysPacket(&egopkt);
  sys_packet_addUint16(&egopkt, NET_CHECK_CRC);
  sys_packet_addUint32(&egopkt, seed);
  sys_packet_addString(&egopkt, source);
  net_sendSysPacketToPeerGuaranteed(peer, &egopkt);

  // Wait for the client to acknowledge the request and to process the message (handled in the network thread)
  // BUT, we don't want an infinite loop, so make sure we don't wait for more than 5 seconds for
  // CRC_ACKNOWLEDGED and 30 seconds for WATING_FOR_CRC

  // wait up to 5 seconds for the client to respond to the request
  net_logf("NET INFO: nfhost_checkCRC() - Waiting for acknowledgement of remote CRC...\n");
  wait_return = net_waitForPacket(nh->asynch, peer, 5000, NET_ACKNOWLEDGE_CRC, NULL);
  if(rv_fail == wait_return || rv_error == wait_return) 
  {
    net_logf("Timed out!\n");
    net_KickOnePlayer(peer);
    return rv_error;
  };
  net_logf("Received\n");

  // After the request is acknowledged, wait up to 30 seconds for the CRC to be returned
  net_logf("NET INFO: nfhost_checkCRC() - Waiting for CRC... ");
  wait_return = net_waitForPacket(nh->asynch, peer, 30000, NET_SEND_CRC, NULL);
  if(rv_fail == wait_return || rv_error == wait_return) 
  {
    net_logf("Timed out!\n");
    net_KickOnePlayer(peer);
    return rv_error;
  }
  else
  {
    usp = (Uint32 *)(buffer + 2);
    temp_CRC = ENET_NET_TO_HOST_32(*usp);
  }

  net_logf("Received. Local CRC == 0x%08x, Remote CRC == 0x%08x\n", CRC, usp);

  return (temp_CRC == CRC) ? rv_succeed : rv_fail;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
nfile_SendQueue * nfile_SendQueue_new(nfile_SendQueue * q)
{
  fprintf( stdout, "nfile_SendQueue_new()\n");

  if(NULL == q || q->initialized) return q;

  memset(q, 0, sizeof(nfile_SendQueue));

  q->initialized = btrue;

  return q;
};

//------------------------------------------------------------------------------
bool_t nfile_SendQueue_delete(nfile_SendQueue * q)
{
  if(NULL == q) return bfalse;

  if(!q->initialized) return btrue;

  // reset the queue
  q->numTransfers = 0;
  q->TransferHead = q->TransferTail = 0;

  q->initialized = bfalse;

  return btrue;
}

//------------------------------------------------------------------------------
nfile_SendQueue * nfile_SendQueue_renew(nfile_SendQueue * q)
{
  nfile_SendQueue_delete(q);
  return nfile_SendQueue_new(q);
}

//------------------------------------------------------------------------------
nfile_ReceiveQueue * nfile_ReceiveQueue_new(nfile_ReceiveQueue * q)
{
  fprintf( stdout, "nfile_ReceiveQueue_new()\n");

  if(NULL == q || q->initialized) return q;

  memset(q, 0, sizeof(nfile_ReceiveQueue));

  q->initialized = btrue;

  return q;
};

//------------------------------------------------------------------------------
bool_t nfile_ReceiveQueue_delete(nfile_ReceiveQueue * q)
{
  if(NULL == q) return bfalse;

  if(!q->initialized) return btrue;

  // reset the queue
  q->numTransfers = 0;
  q->TransferHead = q->TransferTail = 0;

  q->initialized = bfalse;

  return btrue;
}

//------------------------------------------------------------------------------
nfile_ReceiveQueue * nfile_ReceiveQueue_renew(nfile_ReceiveQueue * q)
{
  nfile_ReceiveQueue_delete(q);
  return nfile_ReceiveQueue_new(q);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// nfile module NetHost singleton management
NetHost * nfile_getHost()
{
  // make sure the host exists
  if(NULL == _nfile_host)
  {
    _nfile_Initialize();
  }

  return _nfile_host;
}

//------------------------------------------------------------------------------
retval_t _nfile_Initialize() 
{ 
  if( NULL != _nfile_host) return rv_succeed;

  _nfile_host = NetHost_create( nfhost_HostCallback );
  if(NULL == _nfile_host) return rv_fail;

  if(!_nfile_atexit_registered)
  {
    atexit(_nfile_Quit);
  }

  return _nfile_startUp();
}

//------------------------------------------------------------------------------
void _nfile_Quit(void)
{ 
  if( !_nfile_atexit_registered ) return;
  if( NULL == _nfile_host ) return;

  NetHost_shutDown( _nfile_host );

  // destroy the services
  nfile_SendState_destroy( &_nfile_snd );
  nfile_ReceiveState_destroy( &_nfile_rec );

  // sestroy the host
  NetHost_destroy( &_nfile_host );
}

//------------------------------------------------------------------------------
retval_t _nfile_startUp(void)
{
  NetHost * nh = nfile_getHost();
  if(NULL == nh) return rv_fail;

  NetHost_startUp(nh, NET_EGOBOO_NETWORK_PORT);

  return nfhost_startThreads();
};

//------------------------------------------------------------------------------
retval_t _nfhost_shutDown(void)
{
  NetHost * nh = nfile_getHost();
  if(NULL == nh) return rv_fail;

  return NetHost_shutDown(nh);
}

//------------------------------------------------------------------------------
bool_t nfile_Started()  { return (NULL != _nfile_host) && _nfile_host->nthread.Active; }

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// nfile module nfile_SendState singleton management
nfile_SendState * _nfile_getSend()
{
  // make sure the host exists
  if(NULL == _nfile_snd)
  {
    _nfile_Send_initialize();
  }

  return _nfile_snd;
}

//------------------------------------------------------------------------------
retval_t _nfile_Send_initialize() 
{ 
  if( NULL != _nfile_snd) return rv_succeed;

  _nfile_snd = nfile_SendState_create();
  if(NULL == _nfile_snd) return rv_fail;

  return _nfile_startUp();
}

////------------------------------------------------------------------------------
//retval_t _nfile_SendState_startUp(void)
//{
//  nfile_SendState * snd = _nfile_getSend();
//  if(NULL == snd) return rv_fail;
//
//  NetHost_startUp(snd, NET_EGOBOO_NETWORK_PORT);
//
//  return nfhost_startThreads();
//};
//
////------------------------------------------------------------------------------
//retval_t _nfhost_shutDown(void)
//{
//  nfile_SendState * snd = nfile_getHost();
//  if(NULL == snd) return rv_fail;
//
//  return NetHost_shutDown(snd);
//}

//------------------------------------------------------------------------------
bool_t nfile_SendStarted()  { return (NULL != _nfile_snd) && _nfile_snd->nthread.Active; }

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// nfile module nfile_ReceiveState singleton management
nfile_ReceiveState * _nfile_getReceive()
{
  // make sure the host exists
  if(NULL == _nfile_rec)
  {
    _nfile_Receive_initialize();
  }

  return _nfile_rec;
}

//------------------------------------------------------------------------------
retval_t _nfile_Receive_initialize() 
{ 
  NetHost * nh;
  if( NULL != _nfile_rec) return rv_succeed;

  nh = nfile_getHost();
  if(NULL == nh) return rv_error;

  _nfile_rec = nfile_ReceiveState_create();
  if(NULL == _nfile_rec) return rv_fail;

  return nfile_ReceiveState_startUp(_nfile_rec);
}

////------------------------------------------------------------------------------
//retval_t _nfile_ReceiveState_startUp(void)
//{
//  nfile_ReceiveState * rec = _nfile_getReceive();
//  if(NULL == rec) return rv_fail;
//
//  NetHost_startUp(rec, NET_EGOBOO_NETWORK_PORT);
//
//  return nfhost_startThreads();
//};
//
////------------------------------------------------------------------------------
//retval_t _nfhost_shutDown(void)
//{
//  nfile_ReceiveState * rec = nfile_getHost();
//  if(NULL == rec) return rv_fail;
//
//  return NetHost_shutDown(rec);
//}

//------------------------------------------------------------------------------
bool_t nfile_ReceiveStarted()  { return (NULL != _nfile_rec) && _nfile_rec->nthread.Active; }