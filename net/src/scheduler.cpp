#include "scheduler.h"
#include "logger.h"
#include <data/any.h>

namespace VK {
	namespace Net {
		Scheduler::~Scheduler() {
			cancel_all();
		}

		timer_id_t Scheduler::invoke(timer_period_t delay, timer_cb_t cb) {
			return this->invoke(delay, 0, cb);
		}

		timer_id_t Scheduler::invoke(timer_period_t delay, timer_cb_t cb, any usr_data) {
			return this->invoke(delay, 0, cb, usr_data);
		}

		bool Scheduler::cancel(timer_id_t id) {
			auto it = m_timers.find(id);
			if (m_timers.end() == it) return false;

			it->second->timer.data = it->second;
			uv_close(reinterpret_cast<uv_handle_t*>(&it->second->timer), [](uv_handle_t* handle) {
				auto tr = reinterpret_cast<timer_req_t*>(handle->data);
				delete tr;
			});

			m_timers.erase(it);
			return true;
		}

		void Scheduler::cancel_all() {
			for (auto kv : m_timers) {
				kv.second->timer.data = kv.second;
				uv_close(reinterpret_cast<uv_handle_t*>(&kv.second->timer), [](uv_handle_t* handle) {
					auto tr = reinterpret_cast<timer_req_t*>(handle->data);
					delete tr;
				});
			}

			m_timers.clear();
		}

		timer_id_t Scheduler::invoke(timer_period_t delay, timer_period_t period, timer_cb_t cb) {
			return this->invoke(delay, period, cb, any());
		}

		timer_id_t Scheduler::invoke(timer_period_t delay, timer_period_t period, timer_cb_t cb, any usr_data) {
			int r;
			timer_req_t* treq;
			uv_loop_t* loop;

			treq = new timer_req_t();
			treq->cb = cb;
			treq->id = ++m_seed;
			treq->period = period;
			treq->scheduler = this;
			treq->user_data = usr_data;

			loop = uv_default_loop();

			r = uv_timer_init(loop, &treq->timer);
			if (r) {
				Logger::instance().error("uv_timer_init error : %s\n", uv_strerror(r));
				delete treq;
				return INVALID_TIMER_ID;
			}

			r = uv_timer_start(&treq->timer,
				[](uv_timer_t* handle) {
				auto tr = reinterpret_cast<timer_req_t*>(handle);
				auto fun = tr->cb;

				if (!tr->period)
					tr->scheduler->cancel(tr->id);

				if (fun)
					fun(tr->user_data);
			},
				delay, period);

			if (r) {
				LOG_UV_ERR(r);
				delete treq;
				return INVALID_TIMER_ID;
			}

			m_timers.insert(m_timers.end(), std::make_pair(m_seed, treq));
			return m_seed;
		}
	}
}
