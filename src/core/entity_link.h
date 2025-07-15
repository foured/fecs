#pragma once

#include "types.h"

namespace fecs {

	class entity_link {
	public:
		entity_link(entity_t id)
			: _id(id) { }

		entity_t id() const {
			if (_expired) return error_entity;
			return _id;
		}

		bool expired() const {
			return _expired;
		}

		void expire() {
			_expired = true;
		}

		explicit operator bool() const {
			return !_expired;
		}

		entity_link(const entity_link&) = delete;
		entity_link& operator=(const entity_link&) = delete;

	private:
		entity_t _id;
		bool _expired = false;

	};

}