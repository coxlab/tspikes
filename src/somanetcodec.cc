#include "somanetcodec.h"

SomaNetworkCodec::SomaNetworkCodec(NetworkInterface * pNetwork, int src, 
				   chanproplist_t channels) :
  pNetwork_(pNetwork), 
  dsrc_(src), 
  chanprops_(channels)
{

  Glib::signal_io().connect(sigc::mem_fun(*this, &SomaNetworkCodec::dataRXCallback), 
			    pNetwork_->getDataFifoPipe(), Glib::IO_IN); 
  Glib::signal_io().connect(sigc::mem_fun(*this, &SomaNetworkCodec::eventRXCallback), 
			    pNetwork_->getEventFifoPipe(), Glib::IO_IN); 
  
  channelStateCache_.resize(channels.size()); 

}

SomaNetworkCodec::SomaNetworkCodec(NetworkInterface * pNetwork, int src) :
  pNetwork_(pNetwork), 
  dsrc_(src)
{

  chanprop_t x, y, a,  b; 
  x.chan = 0; 
  x.name = "X"; 
  y.chan = 1; 
  y.name = "Y"; 
  a.chan = 2; 
  a.name = "A"; 
  b.chan = 3; 
  b.name = "B"; 
  chanprops_.push_back(x); 
  chanprops_.push_back(y); 
  chanprops_.push_back(a); 
  chanprops_.push_back(b); 
    
  
  Glib::signal_io().connect(sigc::mem_fun(*this, &SomaNetworkCodec::dataRXCallback), 
			    pNetwork_->getDataFifoPipe(), Glib::IO_IN); 
  Glib::signal_io().connect(sigc::mem_fun(*this, &SomaNetworkCodec::eventRXCallback), 
			    pNetwork_->getEventFifoPipe(), Glib::IO_IN); 
  
  channelStateCache_.resize(chanprops_.size()); 

}



sigc::signal<void, int, TSpikeChannelState> &  SomaNetworkCodec::signalSourceStateChange() {
  return signalSourceStateChange_; 

}

sigc::signal<void, somatime_t> & SomaNetworkCodec::signalTimeUpdate()
{
  return signalTimeUpdate_; 
}

sigc::signal<void, const TSpike_t &> & SomaNetworkCodec::signalNewTSpike()
{
  
  return signalNewTSpike_; 
  
}


void SomaNetworkCodec::processNewData(DataPacket_t * dp)
{
  if (dp->typ == TSPIKE ) {
    TSpike_t newTSpike = rawToTSpike(dp); 
    signalNewTSpike_.emit(newTSpike); 

  } else {
    // somehow we received a datapacket that wasn't for us
  }


}

void SomaNetworkCodec::processNewEvents(EventList_t * pEventList)
{
  EventList_t::iterator pe; 
  for (pe = pEventList->begin(); pe != pEventList->end(); pe++) {
   
      parseEvent(*pe); 
  }
        
}

void SomaNetworkCodec::parseEvent(const Event_t & evt)
{
  
  if (evt.src == 0x00 && evt.cmd == 0x10 )
    {
      // this is the time
      uint64_t time = 0; 
      time = evt.data[0]; 
      time = time << 16; 
      time |= evt.data[1]; 
      time = time << 16; 
      time |= evt.data[2]; 
      float ftime = float(time) / 50e3; 
      
      signalTimeUpdate_.emit(ftime); 
    } 

  bool stateCacheUpdate = false; 
  if (evt.src == dsrc_to_esrc(dsrc_) && evt.cmd  == 0x92) {
    // state update
    int tgtchan = evt.data[0] & 0xFF; 
    STATEPARM param = toStateParm((evt.data[0] >> 8) & 0xFF); 
    
    int pos = -1;
    int i = 0; 
    for (chanproplist_t::iterator c = chanprops_.begin(); 
	 c != chanprops_.end(); c++) 
      {
	if (c->chan == tgtchan) {
	  pos = i; 
	} 
	i++; 
      }
    if (pos > -1 ) {

      // find the appropriate state cache value: 
      
      switch(param) {
      case GAIN: 
	channelStateCache_[pos].gain = evt.data[1]; 
	break; 
      case RANGE: 
	std::cout << "Received update for RANGE" << std::endl; 
	int32_t min, max; 
	min = (evt.data[1] << 16) | (evt.data[2] & 0xFFFF); 
	max = (evt.data[3] << 16) | (evt.data[4] & 0xFFFF); 
	channelStateCache_[pos].rangeMin =  min; 
	channelStateCache_[pos].rangeMax = max; 
	break; 
      default: 
	std::cerr << "Should not get here; unknown property update"
		  << param << std::endl;
      }
      
    } else {
      std::cerr << "This channel update was not for us" 
		<< pos << ' ' << i << std::endl; 
    }
    signalSourceStateChange_.emit(tgtchan, channelStateCache_[pos]); 
    
  }

  // if event is a state update; we do the state-update-dance
  
  // emit relevant signals


}

bool SomaNetworkCodec::dataRXCallback(Glib::IOCondition io_condition)
{
  
  if ((io_condition & Glib::IO_IN) == 0) {
    std::cerr << "Invalid fifo response" << std::endl;
    return false; 
  }
  else 
    {
      char x; 
      read(pNetwork_->getDataFifoPipe(), &x, 1); 
      DataPacket_t * rdp = pNetwork_->getNewData(); 

      processNewData(rdp); 

      delete rdp; 
    
    }
  return true; 
}


bool SomaNetworkCodec::eventRXCallback(Glib::IOCondition io_condition)
{
  
  if ((io_condition & Glib::IO_IN) == 0) {
    std::cerr << "Invalid fifo response" << std::endl;
    return false; 
  }
  else 
    {
      char x; 
      read(pNetwork_->getEventFifoPipe(), &x, 1); 
      EventList_t * pel = pNetwork_->getNewEvents(); 
      processNewEvents(pel); 
      delete pel; 
    }
  return true; 
}


void SomaNetworkCodec::querySourceState(int chan) {
  // send an event to query


  EventTXList_t etl(6); 
  for(int i = 0; i < 6; i++) {
    etl[i].destaddr[dsrc_to_esrc(dsrc_)] = 1; // THIS SOURCE DEVICE
    etl[i].event.src = 4; 
    etl[i].event.cmd = 0x91; 
    etl[i].event.data[0] = chan ; 
  }

  etl[0].event.data[1] = GAIN; 
  etl[1].event.data[1] = THOLD; 
  etl[2].event.data[1] = HPF; 
  etl[3].event.data[1] = FILT; 
  etl[4].event.data[1] = RANGE;
  etl[5].event.data[1] = RANGE; 

  pNetwork_->sendEvents(etl); 

}

void SomaNetworkCodec::refreshStateCache() {
  // send refresh events for all
  chanproplist_t::iterator c; 
  for (c = chanprops_.begin() ; c != chanprops_.end(); c++) {
    querySourceState(c->chan); 
  }

}