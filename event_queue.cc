#include "event_queue.h"
#include "app_context.h"

event_queue::event_queue(int const capacity, app * ac):capacity_(capacity), ac_(ac){
  e_q_.reserve(capacity_);
  std::make_heap(e_q_.begin(),e_q_.end());
}

event_queue::~event_queue(){
  while (!empty())
    consume_event();
  std::cout<<"queue destroyed\n";
}

void event_queue::print_queue_elements() const{
  for (int i = 0; i < e_q_.size(); i++){
    std::string data = e_q_[i]->get_data<std::string>();
    std::cout << data << "\n";
  }
}

int event_queue::size() const {return e_q_.size();}

bool event_queue::empty() const {return e_q_.empty();}

void event_queue::produce_event(std::string && data){
  std::lock_guard<std::mutex> guard(queue_lock_);
  if (e_q_.size()>capacity_)
    return;  
  //std::unique_ptr<event<std::string>> trigger_event = std::make_unique<event<std::string>>(100, std::move(data));
  e_q_.emplace_back(std::make_unique<event<std::string>>(100, std::move(data)));
  std::push_heap(e_q_.begin(),e_q_.end());
  return;
}

std::unique_ptr<base_event> event_queue::consume_event(){
  std::lock_guard<std::mutex> guard(queue_lock_);
  std::unique_ptr<base_event> trigger_event;
  if (!empty()){
    std::pop_heap(e_q_.begin(), e_q_.end());
    trigger_event = std::move(e_q_.back());
    e_q_.pop_back();
  }
  return std::move(trigger_event);
}

int event_queue::get_priority(std::string const action) const{
  int priority=0;
  if (action=="front")
    priority = 100;
  else if (action=="back")
    priority = 30;
  else if (action=="left" || action=="right")
    priority = 15;
  else
    priority = 10;
  return priority;
}

