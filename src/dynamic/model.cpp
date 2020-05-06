﻿#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <limits>
#include <sstream>
#include <regex>
#include <limits>
#include <type_traits>
#include <ios>

#include "aris/core/core.hpp"
#include "aris/dynamic/model.hpp"

namespace aris::dynamic
{
	struct Marker::Imp
	{
		double prt_pm_[4][4]{ { 0 } };
		double pm_[4][4]{ { 0 } };
		Part *part_;
	};
	struct Model::Imp
	{
		double time_{ 0.0 };
		aris::core::Calculator calculator_;
		Environment *environment_;
		Part* ground_;
		aris::core::ObjectPool<Variable, Element> *variable_pool_;
		aris::core::ObjectPool<Part, Element> *part_pool_;
		aris::core::ObjectPool<Joint, Element> *joint_pool_;
		aris::core::ObjectPool<Motion, Element> *motion_pool_;
		aris::core::ObjectPool<GeneralMotion, Element> *general_motion_pool_;
		aris::core::ObjectPool<Force, Element> *force_pool_;
		aris::core::ObjectPool<Solver, Element> *solver_pool_;
		aris::core::ObjectPool<Simulator, Element> *simulator_pool_;
		aris::core::ObjectPool<SimResult, Element> *sim_result_pool_;
		aris::core::ObjectPool<Calibrator, Element> *calibrator_pool_;
	};
	auto Model::loadXml(const aris::core::XmlElement &xml_ele)->void
	{
		Object::loadXml(xml_ele);

		setTime(Object::attributeDouble(xml_ele, "time", 0.0));

		imp_->environment_ = findOrInsertType<Environment>();
		imp_->variable_pool_ = findOrInsertType<aris::core::ObjectPool<Variable, Element>>();
		imp_->part_pool_ = findOrInsertType<aris::core::ObjectPool<Part, Element>>();
		imp_->joint_pool_ = findOrInsertType<aris::core::ObjectPool<Joint, Element>>();
		imp_->motion_pool_ = findOrInsertType<aris::core::ObjectPool<Motion, Element>>();
		imp_->general_motion_pool_ = findOrInsertType<aris::core::ObjectPool<GeneralMotion, Element>>();
		imp_->force_pool_ = findOrInsertType<aris::core::ObjectPool<Force, Element>>();
		imp_->solver_pool_ = findOrInsertType<aris::core::ObjectPool<Solver, Element>>();
		imp_->simulator_pool_ = findOrInsertType<aris::core::ObjectPool<Simulator, Element>>();
		imp_->sim_result_pool_ = findOrInsertType<aris::core::ObjectPool<SimResult, Element>>();
		imp_->calibrator_pool_ = findOrInsertType<aris::core::ObjectPool<Calibrator, Element>>();
		imp_->ground_ = partPool().findOrInsert<Part>("ground");
	}
	auto Model::saveXml(aris::core::XmlElement &xml_ele)const->void
	{
		Object::saveXml(xml_ele);
		xml_ele.SetAttribute("time", time());
	}
	auto Model::init()->void 
	{ 
		auto init_interaction = [](Interaction &interaction, Model*m)->void
		{
			if (interaction.prtNameM().empty() && interaction.prtNameN().empty() && interaction.makNameI().empty() && interaction.makNameJ().empty())return;
			
			auto prt_m = m->partPool().findByName(interaction.prtNameM());
			auto mak_i = prt_m->markerPool().findByName(interaction.makNameI());
			auto prt_n = m->partPool().findByName(interaction.prtNameN());
			auto mak_j = prt_n->markerPool().findByName(interaction.makNameJ());

			interaction.makI_ = &*mak_i;
			interaction.makJ_ = &*mak_j;
		};

		imp_->ground_ = partPool().findOrInsert<Part>("ground");

		environment().model_ = this;
		variablePool().model_ = this;
		for (auto &ele : variablePool())ele.model_ = this;
		partPool().model_ = this;
		for (auto &ele : partPool()) 
		{
			ele.model_ = this;
			for (auto &mak : ele.markerPool())mak.imp_->part_ = &ele;
		}
		jointPool().model_ = this;
		for (auto &ele : jointPool()) 
		{
			ele.model_ = this;
			init_interaction(ele, this);
		}
		motionPool().model_ = this;
		for (auto &ele : motionPool()) {
			ele.model_ = this;
			init_interaction(ele, this);
		}
		generalMotionPool().model_ = this;
		for (auto &ele : generalMotionPool()) {
			ele.model_ = this;
			init_interaction(ele, this);
		}
		forcePool().model_ = this;
		for (auto &ele : forcePool()) {
			ele.model_ = this;
			init_interaction(ele, this);
		}
		solverPool().model_ = this;
		for (auto &ele : solverPool())ele.model_ = this;
		simulatorPool().model_ = this;
		for (auto &ele : simulatorPool())ele.model_ = this;
		simResultPool().model_ = this;
		for (auto &ele : simResultPool())ele.model_ = this;
		calibratorPool().model_ = this;
		for (auto &ele : calibratorPool())ele.model_ = this;

		// alloc mem for solvers //
		for (auto &s : this->solverPool())
			s.allocateMemory();
	}
	auto Model::inverseKinematics()->int { return solverPool()[0].kinPos(); }
	auto Model::forwardKinematics()->int { return solverPool()[1].kinPos(); }
	auto Model::inverseKinematicsVel()->void { solverPool()[0].kinVel(); }
	auto Model::forwardKinematicsVel()->void { solverPool()[1].kinVel(); }
	auto Model::inverseDynamics()->void { solverPool()[2].dynAccAndFce(); }
	auto Model::forwardDynamics()->void { solverPool()[3].dynAccAndFce(); }
	auto Model::setMotionPos(const double *mp)->void { for (Size i = 0; i < motionPool().size(); ++i) motionPool()[i].setMp(mp[i]); }
	auto Model::setMotionPos(double mp, Size which_motion)->void { motionPool()[which_motion].setMp(mp); }
	auto Model::getMotionPos(double *mp)->void { for (Size i = 0; i < motionPool().size(); ++i) mp[i] = motionPool()[i].mp(); }
	auto Model::getMotionPos(Size which_motion)->double { return motionPool()[which_motion].mp(); }
	auto Model::setMotionVel(const double *mv)->void { for (Size i = 0; i < motionPool().size(); ++i) motionPool()[i].setMv(mv[i]); }
	auto Model::setMotionVel(double mv, Size which_motion)->void { motionPool()[which_motion].setMv(mv); }
	auto Model::getMotionVel(double *mv)->void { for (Size i = 0; i < motionPool().size(); ++i) mv[i] = motionPool()[i].mv(); }
	auto Model::getMotionVel(Size which_motion)->double { return motionPool()[which_motion].mv(); }
	auto Model::setMotionFce(const double *mf)->void { for (Size i = 0; i < motionPool().size(); ++i) motionPool()[i].setMf(mf[i]); }
	auto Model::setMotionFce(double mf, Size which_motion)->void { motionPool()[which_motion].setMf(mf); }
	auto Model::getMotionFce(double *mf)->void { for (Size i = 0; i < motionPool().size(); ++i) mf[i] = motionPool()[i].mf(); }
	auto Model::getMotionFce(Size which_motion)->double { return motionPool()[which_motion].mf(); }
	auto Model::setEndEffectorPm(const double *pm, Size which_ee)->void { generalMotionPool()[which_ee].setMpm(pm); }
	auto Model::getEndEffectorPm(double *pm, Size which_ee)->void { generalMotionPool()[which_ee].getMpm(pm); }
	auto Model::time()const->double { return imp_->time_; }
	auto Model::setTime(double time)->void { imp_->time_ = time; }
	auto Model::calculator()->aris::core::Calculator& { return imp_->calculator_; }
	auto Model::environment()->aris::dynamic::Environment& { return *imp_->environment_; }
	auto Model::variablePool()->aris::core::ObjectPool<Variable, Element>& { return *imp_->variable_pool_; }
	auto Model::partPool()->aris::core::ObjectPool<Part, Element>& { return *imp_->part_pool_; }
	auto Model::jointPool()->aris::core::ObjectPool<Joint, Element>& { return *imp_->joint_pool_; }
	auto Model::motionPool()->aris::core::ObjectPool<Motion, Element>& { return *imp_->motion_pool_; }
	auto Model::generalMotionPool()->aris::core::ObjectPool<GeneralMotion, Element>& { return *imp_->general_motion_pool_; }
	auto Model::forcePool()->aris::core::ObjectPool<Force, Element>& { return *imp_->force_pool_; }
	auto Model::solverPool()->aris::core::ObjectPool<Solver, Element>& { return *imp_->solver_pool_; }
	auto Model::simulatorPool()->aris::core::ObjectPool<Simulator, Element>& { return *imp_->simulator_pool_; }
	auto Model::simResultPool()->aris::core::ObjectPool<SimResult, Element>& { return *imp_->sim_result_pool_; }
	auto Model::calibratorPool()->aris::core::ObjectPool<Calibrator, Element>& { return *imp_->calibrator_pool_; }
	auto Model::ground()->Part& { return *imp_->ground_; }
	auto Model::addPartByPm(const double*pm, const double *prt_im)->Part& 
	{ 
		auto &ret = partPool().add<Part>("part_" + std::to_string(partPool().size()), prt_im, pm);
		ret.Element::model_ = this;
		return ret;
	}
	auto Model::addPartByPe(const double*pe, const char* eul_type, const double *prt_im)->Part&
	{
		double pm[16];
		s_pe2pm(pe, pm, eul_type);
		auto &ret = partPool().add<Part>("part_" + std::to_string(partPool().size()), prt_im, pm);
		ret.Element::model_ = this;
		return ret;
	}
	auto Model::addPartByPq(const double*pq, const double *prt_im)->Part&
	{
		double pm[16];
		s_pq2pm(pq, pm);
		auto &ret = partPool().add<Part>("part_" + std::to_string(partPool().size()), prt_im, pm);
		ret.Element::model_ = this;
		return ret;
	}
	auto Model::addRevoluteJoint(Part &first_part, Part &second_part, const double *position, const double *axis)->RevoluteJoint&
	{
		double glb_pm[16], loc_pm[16];
		s_sov_axes2pm(position, axis, axis, glb_pm, "zx");
		auto name = "joint_" + std::to_string(jointPool().size());
		s_inv_pm_dot_pm(*first_part.pm(), glb_pm, loc_pm);
		auto &mak_i = first_part.addMarker(name + "_i", loc_pm);
		s_inv_pm_dot_pm(*second_part.pm(), glb_pm, loc_pm);
		auto &mak_j = second_part.addMarker(name + "_j", loc_pm);

		auto &ret = jointPool().add<RevoluteJoint>(name, &mak_i, &mak_j);
		ret.Element::model_ = this;
		return ret;
	}
	auto Model::addPrismaticJoint(Part &first_part, Part &second_part, const double *position, const double *axis)->PrismaticJoint&
	{
		double glb_pm[16], loc_pm[16];
		s_sov_axes2pm(position, axis, axis, glb_pm, "zx");
		auto name = "joint_" + std::to_string(jointPool().size());
		s_inv_pm_dot_pm(*first_part.pm(), glb_pm, loc_pm);
		auto &mak_i = first_part.addMarker(name + "_i", loc_pm);
		s_inv_pm_dot_pm(*second_part.pm(), glb_pm, loc_pm);
		auto &mak_j = second_part.addMarker(name + "_j", loc_pm);

		auto &ret = jointPool().add<PrismaticJoint>(name, &mak_i, &mak_j);
		ret.Element::model_ = this;
		return ret;
	}
	auto Model::addUniversalJoint(Part &first_part, Part &second_part, const double *position, const double *first_axis, const double *second_axis)->UniversalJoint&
	{
		double glb_pm[16], loc_pm[16];
		s_sov_axes2pm(position, first_axis, second_axis, glb_pm, "zx");
		auto name = "joint_" + std::to_string(jointPool().size());
		s_inv_pm_dot_pm(*first_part.pm(), glb_pm, loc_pm);
		auto &mak_i = first_part.addMarker(name + "_i", loc_pm);
		s_sov_axes2pm(position, second_axis, first_axis, glb_pm, "zx");
		s_inv_pm_dot_pm(*second_part.pm(), glb_pm, loc_pm);
		auto &mak_j = second_part.addMarker(name + "_j", loc_pm);

		auto &ret = jointPool().add<UniversalJoint>(name, &mak_i, &mak_j);
		ret.Element::model_ = this;
		return ret;
	}
	auto Model::addSphericalJoint(Part &first_part, Part &second_part, const double *position)->SphericalJoint&
	{
		double glb_pm[16]{ 1,0,0,position[0],0,1,0,position[1],0,0,1,position[2],0,0,0,1 }, loc_pm[16];
		auto name = "joint_" + std::to_string(jointPool().size());
		s_inv_pm_dot_pm(*first_part.pm(), glb_pm, loc_pm);
		auto &mak_i = first_part.addMarker(name + "_i", loc_pm);
		s_inv_pm_dot_pm(*second_part.pm(), glb_pm, loc_pm);
		auto &mak_j = second_part.addMarker(name + "_j", loc_pm);
		auto &ret = jointPool().add<SphericalJoint>(name, &mak_i, &mak_j);
		ret.Element::model_ = this;
		return ret;
	}
	auto Model::addMotion(Joint &joint)->Motion&
	{
		Size dim;

		if (dynamic_cast<RevoluteJoint*>(&joint))
		{
			dim = 5;
		}
		else if (dynamic_cast<PrismaticJoint*>(&joint))
		{
			dim = 2;
		}
		else
		{
			THROW_FILE_LINE("wrong joint when Model::addMotion(joint)");
		}

		auto &ret = motionPool().add<Motion>("motion_" + std::to_string(motionPool().size()), joint.makI(), joint.makJ(), dim);
		ret.Element::model_ = this;
		return ret;
	}
	auto Model::addMotion()->Motion&
	{
		if (ground().markerPool().findByName("origin") == ground().markerPool().end())
		{
			double pm[16] = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
			this->ground().addMarker("origin", pm);
		}

		auto mak = ground().markerPool().findByName("origin");

		auto &ret = motionPool().add<Motion>("motion_" + std::to_string(motionPool().size()), &*mak, &*mak, 0);
		ret.Element::model_ = this;
		return ret;
	}
	auto Model::addGeneralMotionByPm(Part &end_effector, Coordinate &reference, const double* pm)->GeneralMotion&
	{
		double pm_prt[16], pm_target_in_ground[16];
		s_pm_dot_pm(*reference.pm(), pm, pm_target_in_ground);
		s_inv_pm_dot_pm(*end_effector.pm(), pm_target_in_ground, pm_prt);

		auto name = "general_motion_" + std::to_string(generalMotionPool().size());
		auto &mak_i = end_effector.addMarker(name + "_i", pm_prt);
		auto &mak_j = dynamic_cast<Part*>(&reference) ? dynamic_cast<Part&>(reference).addMarker(name + "_j") : dynamic_cast<Marker&>(reference);
		
		auto &ret = generalMotionPool().add<GeneralMotion>(name, &mak_i, &mak_j);
		ret.Element::model_ = this;
		return ret;
	}
	auto Model::addGeneralMotionByPe(Part &end_effector, Coordinate &reference, const double* pe, const char* eul_type)->GeneralMotion&
	{
		auto &ret = addGeneralMotionByPm(end_effector, reference, s_pe2pm(pe, nullptr, eul_type));
		ret.Element::model_ = this;
		return ret;
	}
	auto Model::addGeneralMotionByPq(Part &end_effector, Coordinate &reference, const double* pq)->GeneralMotion&
	{
		auto &ret = addGeneralMotionByPm(end_effector, reference, s_pq2pm(pq));
		ret.Element::model_ = this;
		return ret;
	}
	Model::~Model() = default;
	Model::Model(const std::string &name) :Object(name)
	{
		this->registerType<aris::core::ObjectPool<Variable, Element> >();
		this->registerType<aris::core::ObjectPool<Part, Element> >();
		this->registerType<aris::core::ObjectPool<Joint, Element> >();
		this->registerType<aris::core::ObjectPool<Motion, Element> >();
		this->registerType<aris::core::ObjectPool<GeneralMotion, Element> >();
		this->registerType<aris::core::ObjectPool<Force, Element> >();
		this->registerType<aris::core::ObjectPool<Solver, Element> >();
		this->registerType<aris::core::ObjectPool<Simulator, Element> >();
		this->registerType<aris::core::ObjectPool<SimResult, Element> >();
		this->registerType<aris::core::ObjectPool<Calibrator, Element> >();

		imp_->environment_ = &this->add<Environment>("environment");
		imp_->variable_pool_ = &this->add<aris::core::ObjectPool<Variable, Element>>("variable_pool");
		imp_->part_pool_ = &this->add<aris::core::ObjectPool<Part, Element>>("part_pool");
		imp_->joint_pool_ = &this->add<aris::core::ObjectPool<Joint, Element>>("joint_pool");
		imp_->motion_pool_ = &this->add<aris::core::ObjectPool<Motion, Element>>("motion_pool");
		imp_->general_motion_pool_ = &this->add<aris::core::ObjectPool<GeneralMotion, Element>>("general_motion_pool");
		imp_->force_pool_ = &this->add<aris::core::ObjectPool<Force, Element>>("force_pool");
		imp_->solver_pool_ = &this->add<aris::core::ObjectPool<Solver, Element>>("solver_pool");
		imp_->simulator_pool_ = &this->add<aris::core::ObjectPool<Simulator, Element>>("simulator_pool");
		imp_->sim_result_pool_ = &this->add<aris::core::ObjectPool<SimResult, Element>>("sim_result_pool");
		imp_->calibrator_pool_ = &this->add<aris::core::ObjectPool<Calibrator, Element>>("calibrator_pool");

		imp_->ground_ = &imp_->part_pool_->add<Part>("ground");
	}
	ARIS_DEFINE_BIG_FOUR_CPP(Model);

	ARIS_REGISTRATION
	{
		typedef Environment&(Model::*EnvironmentFunc)();
		typedef aris::core::ObjectPool<Variable, Element> &(Model::*VarablePoolFunc)();
		typedef aris::core::ObjectPool<Part, Element> &(Model::*PartPoolFunc)();
		typedef aris::core::ObjectPool<Joint, Element> &(Model::*JointPoolFunc)();
		typedef aris::core::ObjectPool<Motion, Element> &(Model::*MotionPoolFunc)();
		typedef aris::core::ObjectPool<GeneralMotion, Element> &(Model::*GeneralMotionPoolFunc)();
		typedef aris::core::ObjectPool<Force, Element> &(Model::*ForcePoolFunc)();
		typedef aris::core::ObjectPool<Solver, Element> &(Model::*SolverPoolFunc)();
		typedef aris::core::ObjectPool<Simulator, Element> &(Model::*SimulatorPoolFunc)();
		typedef aris::core::ObjectPool<SimResult, Element> &(Model::*SimResultPoolFunc)();
		typedef aris::core::ObjectPool<Calibrator, Element> &(Model::*CalibratorPoolFunc)();

		aris::core::class_<aris::core::ObjectPool<Variable, Element>>("VariablePoolElement")
			.asRefArray()
			;
		aris::core::class_<aris::core::ObjectPool<Part, Element>>("PartPoolElement")
			.asRefArray()
			;
		aris::core::class_<aris::core::ObjectPool<Joint, Element>>("JointPoolElement")
			.asRefArray()
			;
		aris::core::class_<aris::core::ObjectPool<Motion, Element>>("MotionPoolElement")
			.asRefArray()
			;
		aris::core::class_<aris::core::ObjectPool<GeneralMotion, Element>>("GeneralMotionPoolElement")
			.asRefArray()
			;
		aris::core::class_<aris::core::ObjectPool<Force, Element>>("ForcePoolElement")
			.asRefArray()
			;
		aris::core::class_<aris::core::ObjectPool<Solver, Element>>("SolverPoolElement")
			.asRefArray()
			;
		aris::core::class_<aris::core::ObjectPool<Simulator, Element>>("SimulatorPoolElement")
			.asRefArray()
			;
		aris::core::class_<aris::core::ObjectPool<SimResult, Element>>("SimResultPoolElement")
			.asRefArray()
			;
		aris::core::class_<aris::core::ObjectPool<Calibrator, Element>>("CalibratorPoolElement")
			.asRefArray()
			;

		aris::core::class_<Model>("Model")
			.inherit<aris::core::Object>()
			.property("time", &Model::setTime, &Model::time)
			.property<EnvironmentFunc>("environment", &Model::environment)
			.property<VarablePoolFunc>("variable_pool", &Model::variablePool)
			.property<PartPoolFunc>("part_pool", &Model::partPool)
			.property<MotionPoolFunc>("motion_pool", &Model::motionPool)
			.property<JointPoolFunc>("joint_pool", &Model::jointPool)
			.property<GeneralMotionPoolFunc>("general_motion_pool", &Model::generalMotionPool)
			.property<ForcePoolFunc>("force_pool", &Model::forcePool)
			.property<SolverPoolFunc>("solver_pool", &Model::solverPool)
			//.property<SimulatorPoolFunc>("simulator_pool", &Model::simulatorPool)
			//.property<SimResultPoolFunc>("sim_result_pool", &Model::simResultPool)
			//.property<CalibratorPoolFunc>("calibrator_pool", &Model::calibratorPool)
			;
	}

}
