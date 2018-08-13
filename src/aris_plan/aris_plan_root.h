﻿#ifndef ARIS_PLAN_ROOT_H_
#define ARIS_PLAN_ROOT_H_

#include <list>
#include <cmath>
#include <iostream>
#include <functional>
#include <map>

#include <aris_core.h>

namespace aris
{
	namespace dynamic { class Model; class SimResult; }
	namespace control { class Master; }
	
	
	/// \brief 轨迹规划命名空间
	/// \ingroup aris
	/// 
	///
	///
	namespace plan 
	{
		struct PlanParam
		{
			std::uint32_t count_;
			aris::dynamic::Model* model_;
			aris::control::Master* master_;
			void *param_;
			std::uint32_t param_size_;
		};
		class Plan:public aris::core::Object
		{
		public:
			enum Option : std::uint64_t
			{
				NOT_RUN_PREPAIR_FUNCTION = 0x01ULL << 0,
				PREPAIR_WHEN_ALL_PLAN_EXECUTED = 0x01ULL << 1,
				PREPAIR_WHEN_ALL_PLAN_COLLECTED = 0x01ULL << 2,
								
				NOT_RUN_EXECUTE_FUNCTION = 0x01ULL << 3,
				EXECUTE_WHEN_ALL_PLAN_EXECUTED = 0x01ULL << 4,
				EXECUTE_WHEN_ALL_PLAN_COLLECTED = 0x01ULL << 5,
				WAIT_FOR_EXECUTION = 0x01ULL << 6,
				WAIT_IF_CMD_POOL_IS_FULL = 0x01ULL << 7,

				NOT_RUN_COLLECT_FUNCTION = 0x01ULL << 8,
				COLLECT_WHEN_ALL_PLAN_EXECUTED = 0x01ULL << 9,
				COLLECT_WHEN_ALL_PLAN_COLLECTED = 0x01ULL << 10,
				WAIT_FOR_COLLECTION = 0x01ULL << 11,

				USING_TARGET_POS = 0x01ULL << 16,
				USING_TARGET_VEL = 0x01ULL << 17,
				USING_TARGET_CUR = 0x01ULL << 18,
				USING_VEL_OFFSET = 0x01ULL << 19,
				USING_CUR_OFFSET = 0x01ULL << 20,

				NOT_CHECK_POS_MIN = 0x01ULL << 24,
				NOT_CHECK_POS_MAX = 0x01ULL << 25,
				NOT_CHECK_POS_CONTINUOUS = 0x01ULL << 26,
				NOT_CHECK_POS_CONTINUOUS_AT_START = 0x01ULL << 27,
				NOT_CHECK_POS_CONTINUOUS_SECOND_ORDER = 0x01ULL << 28,
				NOT_CHECK_POS_CONTINUOUS_SECOND_ORDER_AT_START = 0x01ULL << 29,
				NOT_CHECK_POS_FOLLOWING_ERROR = 0x01ULL << 30,

				NOT_CHECK_VEL_MIN = 0x01ULL << 31,
				NOT_CHECK_VEL_MAX = 0x01ULL << 32,
				NOT_CHECK_VEL_CONTINUOUS = 0x01ULL << 33,
				NOT_CHECK_VEL_CONTINUOUS_AT_START = 0x01ULL << 34,
				NOT_CHECK_VEL_FOLLOWING_ERROR = 0x01ULL << 35,
			};

			static auto Type()->const std::string &{ static const std::string type("Plan"); return std::ref(type); }
			auto virtual type() const->const std::string& override{ return Type(); }
			auto virtual prepairNrt(const PlanParam &param, const std::map<std::string,std::string> &params, aris::core::Msg &msg_out)->void {}
			auto virtual executeRT(const PlanParam &param)->int { return 0; }
			auto virtual collectNrt(const PlanParam &param)->void {}
			auto command()->aris::core::Command &;
			auto command()const->const aris::core::Command & { return const_cast<std::decay_t<decltype(*this)> *>(this)->command(); }

			virtual ~Plan();
			explicit Plan(const std::string &name = "plan");
			Plan(const Plan &);
			Plan(Plan &&);
			Plan& operator=(const Plan &);
			Plan& operator=(Plan &&);

		private:
			struct Imp;
			aris::core::ImpPtr<Imp> imp_;
		};
		class PlanRoot :public aris::core::Object
		{
		public:
			static auto Type()->const std::string &{ static const std::string type("PlanRoot"); return std::ref(type); }
			auto virtual type() const->const std::string& override{ return Type(); }
			auto planPool()->aris::core::ObjectPool<Plan> &;
			auto planPool()const->const aris::core::ObjectPool<Plan> &{ return const_cast<std::decay_t<decltype(*this)> *>(this)->planPool(); }
			auto planParser()->aris::core::CommandParser;

			virtual ~PlanRoot();
			explicit PlanRoot(const std::string &name = "plan_root");
			PlanRoot(const PlanRoot &);
			PlanRoot(PlanRoot &&);
			PlanRoot& operator=(const PlanRoot &);
			PlanRoot& operator=(PlanRoot &&);

		private:
			struct Imp;
			aris::core::ImpPtr<Imp> imp_;
		};

		class EnablePlan : public Plan
		{
		public:
			static auto Type()->const std::string & { static const std::string type("EnablePlan"); return std::ref(type); }
			auto virtual type() const->const std::string& override { return Type(); }
			auto virtual prepairNrt(const PlanParam &param, const std::map<std::string, std::string> &cmd_params, aris::core::Msg &msg_out)->void override;
			auto virtual executeRT(const PlanParam &param)->int override;
			auto virtual collectNrt(const PlanParam &param)->void override;

			virtual ~EnablePlan();
			explicit EnablePlan(const std::string &name = "enable_plan");
			EnablePlan(const EnablePlan &);
			EnablePlan(EnablePlan &&);
			EnablePlan& operator=(const EnablePlan &);
			EnablePlan& operator=(EnablePlan &&);

		private:
			struct Imp;
			aris::core::ImpPtr<Imp> imp_;
		};
		class DisablePlan : public Plan
		{

		};
		class HomePlan : public Plan
		{
		public:
			static auto Type()->const std::string & { static const std::string type("DisablePlan"); return std::ref(type); }
			auto virtual type() const->const std::string& override { return Type(); }
			auto virtual prepairNrt(const PlanParam &param, const std::map<std::string, std::string> &cmd_params, aris::core::Msg &msg_out)->void override;
			auto virtual executeRT(const PlanParam &param)->int override;
			auto virtual collectNrt(const PlanParam &param)->void override;

			virtual ~HomePlan();
			explicit HomePlan(const std::string &name = "disable_plan");
			HomePlan(const HomePlan &);
			HomePlan(HomePlan &&);
			HomePlan& operator=(const HomePlan &);
			HomePlan& operator=(HomePlan &&);

		private:
			struct Imp;
			aris::core::ImpPtr<Imp> imp_;
		};
		class RecoverPlan : public Plan
		{
		public:
			static auto Type()->const std::string & { static const std::string type("RecoverPlan"); return std::ref(type); }
			auto virtual type() const->const std::string& override { return Type(); }
			auto virtual prepairNrt(const PlanParam &param, const std::map<std::string, std::string> &cmd_params, aris::core::Msg &msg_out)->void override;
			auto virtual executeRT(const PlanParam &param)->int override;
			auto virtual collectNrt(const PlanParam &param)->void override;

			virtual ~RecoverPlan();
			explicit RecoverPlan(const std::string &name = "recover_plan");
			RecoverPlan(const RecoverPlan &);
			RecoverPlan(RecoverPlan &&);
			RecoverPlan& operator=(const RecoverPlan &);
			RecoverPlan& operator=(RecoverPlan &&);

		private:
			struct Imp;
			aris::core::ImpPtr<Imp> imp_;
		};
		class MovePlan : public Plan
		{
		public:
			static auto Type()->const std::string & { static const std::string type("MovePlan"); return std::ref(type); }
			auto virtual type() const->const std::string& override { return Type(); }
			auto virtual prepairNrt(const PlanParam &param, const std::map<std::string, std::string> &cmd_params, aris::core::Msg &msg_out)->void override;
			auto virtual executeRT(const PlanParam &param)->int override;
			auto virtual collectNrt(const PlanParam &param)->void override;

			virtual ~MovePlan();
			explicit MovePlan(const std::string &name = "move_plan");
			MovePlan(const MovePlan &);
			MovePlan(MovePlan &&);
			MovePlan& operator=(const MovePlan &);
			MovePlan& operator=(MovePlan &&);

		private:
			struct Imp;
			aris::core::ImpPtr<Imp> imp_;
		};
		class UniversalPlan :public Plan
		{
		public:
			using PrepairFunc = std::function<void(const PlanParam &param, const std::map<std::string, std::string> &cmd_params, aris::core::Msg &msg_out)>;
			using ExecuteFunc = std::function<int(const PlanParam &param)>;
			using CollectFunc = std::function<void(const PlanParam &param)>;

			static auto Type()->const std::string & { static const std::string type("UniversalPlan"); return std::ref(type); }
			auto virtual type() const->const std::string& override { return Type(); }
			auto virtual prepairNrt(const PlanParam &param, const std::map<std::string, std::string> &cmd_params, aris::core::Msg &msg_out)->void override;
			auto virtual executeRT(const PlanParam &param)->int override;
			auto virtual collectNrt(const PlanParam &param)->void override;
			auto virtual setPrepairFunc(PrepairFunc func)->void;
			auto virtual setExecuteFunc(ExecuteFunc func)->void;
			auto virtual setCollectFunc(CollectFunc func)->void;

			virtual ~UniversalPlan();
			explicit UniversalPlan(const std::string &name = "universal_plan", PrepairFunc prepair_func = nullptr, ExecuteFunc execute_func = nullptr, CollectFunc collect_func = nullptr, const std::string & cmd_xml_str = "<universal_plan/>");
			UniversalPlan(const UniversalPlan &);
			UniversalPlan(UniversalPlan &&);
			UniversalPlan& operator=(const UniversalPlan &);
			UniversalPlan& operator=(UniversalPlan &&);

		private:
			struct Imp;
			aris::core::ImpPtr<Imp> imp_;
		};
	}
}


#endif