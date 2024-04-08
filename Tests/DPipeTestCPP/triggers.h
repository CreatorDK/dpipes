#pragma once
#include <functional>

namespace crdk {
	namespace triggers {

		enum TRIGGER_STATUS
		{
			UNSTARTED = -1,
			WAITING = 0,
			COMPLETE = 1,
			TIMEOUT = 2,
			CONDITION_WRONG = 3
		};

		class Trigger {
		protected:
			int _status = -1;
		public:
			bool IsComplete() const;
			bool IsSucces() const;
		};

		void wait(Trigger& trigger, unsigned int timeout = 0);

		class BoolTrigger : public Trigger {
		public:
			void SetComplete();
		};

		bool EQUAL_COMPARATOR(int current, int target);

		bool MORE_COMPARATOR(int current, int target);

		bool MORE_OR_EQUAL_COMPARATOR(int current, int target);

		bool LESS_COMPARATOR(int current, int target);

		bool LESS_OR_EQUAL_COMPARATOR(int current, int target);

		class IntTrigger : public Trigger {
		private:
			int _target = 0;
			int _current = 0;
			std::function<bool(int, int)> _comparator;
		public:
			IntTrigger(int targetNumber, std::function<bool(int, int)> comparator = EQUAL_COMPARATOR);
			IntTrigger(int startNumber, int targetNumber, std::function<bool(int, int)> comparator = EQUAL_COMPARATOR);
			void Increase(int number);
			void Decrease(int number);
		};
	}
}