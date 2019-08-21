﻿#include <iostream>
#include <aris.hpp>

using namespace aris::dynamic;
using namespace aris::robot;

//系统传递函数H(s)=1/(ms)
void PIDcalOne(double m, double ts, double *KP)
{
	double T = ts / 3.0;
	KP[0] = m / T;
}

//系统传递函数H(s)=1/(ms+h)
void PIDcalTeo(double m, double h, double ts, double overshoot, double *KP, double *KI)
{
	double temp = log(overshoot);
	double kesi = 1 / sqrt(1 + aris::PI*aris::PI / temp / temp);
	double omega = 4 / kesi / ts;

	KI[0] = omega * omega * m;
	KP[0] = 2 * kesi * omega * m - h;
}

int main(int argc, char *argv[])
{
	double robot_pm[16];
	std::string robot_name = argc < 2 ? "rokae_xb4" : argv[1];
	auto port = argc < 3 ? 5866 : std::stoi(argv[2]);
	aris::dynamic::s_pq2pm(argc < 4 ? nullptr : aris::core::Calculator().calculateExpression(argv[3]).data(), robot_pm);

	auto&cs = aris::server::ControlServer::instance();
	cs.setName(robot_name);
	if (robot_name == "ur5")
	{
		cs.resetController(createControllerUr5().release());
		cs.resetModel(createModelUr5(robot_pm).release());
		cs.resetPlanRoot(createPlanRootUr5().release());
		cs.resetSensorRoot(new aris::sensor::SensorRoot);
	}
	else if (robot_name == "rokae_xb4")
	{
		cs.resetController(createControllerRokaeXB4().release());
		cs.resetModel(aris::robot::createModelRokaeXB4(robot_pm).release());
		cs.resetPlanRoot(createPlanRootRokaeXB4().release());
		cs.resetSensorRoot(new aris::sensor::SensorRoot);
		cs.interfaceRoot().loadXmlStr(aris::robot::createRokaeXB4Interface());
	}
	else if (robot_name == "servo_press")
	{
		cs.resetController(createControllerServoPress().release());
		cs.resetModel(aris::robot::createModelServoPress(robot_pm).release());
		cs.resetPlanRoot(createPlanRootServoPress().release());
		cs.resetSensorRoot(new aris::sensor::SensorRoot);
	}
	else if (robot_name == "stewart")
	{
		cs.resetController(createControllerStewart().release());
		cs.resetModel(aris::robot::createModelStewart(robot_pm).release());
		cs.resetPlanRoot(createPlanRootStewart().release());
		cs.resetSensorRoot(new aris::sensor::SensorRoot);
		cs.interfaceRoot().loadXmlStr(aris::robot::createRokaeXB4Interface());

		// init model pos //
		//cs.model().generalMotionPool()[0].setMpe(std::array<double, 6>{0, 0, 0.5, 0, 0, 0}.data(), "313");
		//cs.model().solverPool()[0].kinPos();

		//cs.saveXmlFile("C:\\Users\\py033\\Desktop\\stewart.xml");
		
		//cs.loadXmlFile(ARIS_INSTALL_PATH + std::string("/resource/demo_server/stewart.xml"));
	}
	else
	{
		std::cout << "unknown robot:" << robot_name << std::endl;
		return -1;
	}
	std::cout << "this server robot   :" << robot_name << std::endl;
	std::cout << "this server port    :" << std::to_string(port) << std::endl;
	std::cout << "this server position:" << std::endl;
	dsp(4, 4, robot_pm);

	//double KPP[7] = { 200,200,8,-15,-15,-15,0 };
	//double KPV[7] = { 100,100,0,4,4,4,0 };
	//double KIV[7] = { 50,50,  0,1,1,1,0 };

	auto &m = cs.model();
	double mp[6]{ 0,0,0,0,1.57,0 };
	double mv[6]{ 0.001,0.02,0.01,0.04,0.01,0.02 };
	double ma[6]{ 0.1,0.2,0.3,0.4,0.5,0.6 };
	for (auto &mot : cs.model().motionPool())
	{
		mot.setMp(mp[mot.id()]);
		mot.setMv(mv[mot.id()]);
		mot.setMa(ma[mot.id()]);
	} 

	auto &s = dynamic_cast<aris::dynamic::ForwardKinematicSolver&>(m.solverPool()[1]);
	s.kinPos();
	s.kinVel();
	s.dynAccAndFce();
	s.cptGeneralInverseDynamicMatrix();
	s.cptJacobi();
	
	// clear frc //
	for (int i = 0; i < 6; ++i) 
	{
		double frc[3]{0,0,0};
		frc[2] = m.motionPool()[i].frcCoe()[2];
		m.motionPool()[i].setFrcCoe(frc);
	}

	// J_inv
	double U[36], tau[6], J_inv[36], tau2[6];
	aris::Size p[6], rank;
	s_householder_utp(6, 6, s.Jf(), U, tau, p, rank, 1e-7);
	s_householder_utp2pinv(6, 6, rank, U, tau, p, J_inv, tau2, 1e-7);

	// M = (M + I) * J_inv 
	double M[36], A[36], tem[36];
	s_mc(6, 6, s.M(), s.nM(), M, 6);
	for (int i = 0; i < 6; ++i)M[at(i, i, 6)] += m.motionPool()[i].frcCoe()[2];
	s_mm(6, 6, 6, M, J_inv, tem);
	s_mm(6, 6, 6, J_inv, T(6), tem, 6, A, 6);

	// cout torque 
	double mf[6];
	for (int i = 0; i < 6; ++i)tem[i] = m.motionPool()[i].mf();
	s_mm(6, 1, 6, J_inv, T(6), tem, 1, mf, 1);
	dsp(1, 6, mf);

	// h = -M * c + h
	double h[6];
	s_vc(6, s.h(), tem);
	s_mm(6, 1, 6, J_inv, T(6), tem, 1, h, 1);
	s_mms(6, 1, 6, A, s.cf(), h);
	double ee_as[6];
	m.generalMotionPool()[0].getMas(ee_as);
	s_mma(6, 1, 6, A, ee_as, h);
	dsp(1, 6, h);

	// 
	double max_value[6]{ 0,0,0,0,0,0 };
	double f2c_index[6] = { 9.07327526291993, 9.07327526291993, 17.5690184835913, 39.0310903520972, 66.3992503259041, 107.566785527965 };
	for (int i = 0; i < 6; ++i)
	{
		//s_nv(6, f2c_index[i], A + i * 6);
		
		for (int j = 0; j < 6; ++j)
		{
			max_value[j] = std::max(max_value[j], std::abs(A[at(i, j, 6)]));
		}
	}

	dsp(1, 6, max_value);

	double kpp[6];
	double kpv[6], kiv[6];
	for (int i = 0; i < 6; ++i)
	{
		PIDcalOne(max_value[i], 0.2, kpp + i);
		PIDcalTeo(max_value[i], 0, 0.4, 0.0433, kpv + i, kiv + i);
	}

	dsp(1, 6, kpp);
	dsp(1, 6, kpv);
	dsp(1, 6, kiv);

	double ft[6]{ 0,0,0,0,15,0 };
	double JoinTau[6];
	s_mm(6, 1, 6, s.Jf(), T(6), ft, 1, JoinTau, 1);

	dsp(1, 6, JoinTau);


	for (int i = 0; i < 6; ++i)
	{
		JoinTau[i] *= f2c_index[i];
	}

	dsp(1, 6, JoinTau);


	//for (int i = 0; i < 6; ++i)
	//{
	//	max_value[i] *= f2c_index[i];
	//}
	//
	//dsp(1, 6, max_value);



	



	//dsp(sd.nM(), sd.nM(), sd.M());
	//dsp(s.nM(), s.nM(), s.M());

	s.Jf();
	s.M();






	////////////////////////////////////////////////////////////////////////////////////
	//aris::dynamic::SevenAxisParam param;

	//param.d1 = 0.3705;
	//param.d3 = 0.330;
	//param.d5 = 0.320;
	//param.tool0_pe[2] = 0.2205;

	//auto m = aris::dynamic::createModelSevenAxis(param);
	//cs.resetModel(m.release());
	//dynamic_cast<aris::control::EthercatMotion&>(cs.controller().slaveAtAbs(1)).setMinPos(-0.1);
	//dynamic_cast<aris::control::EthercatMotion&>(cs.controller().slaveAtAbs(1)).setMaxPos(0.1);
	////////////////////////////////////////////////////////////////////////////////////

	cs.planRoot().planPool().add<aris::plan::UniversalPlan>("tt", [&](const std::map<std::string, std::string> &, aris::plan::PlanTarget &t)->void
	{
		auto ct = cs.currentCollectTarget();

		t.ret = std::vector<std::pair<std::string, std::any>>();

		if (ct)
		{
			std::cout << "current plan:" << ct->plan->name() << std::endl;
		}
		else
		{
			std::cout << "no current plan" << std::endl;
		}
		
		//t.option = aris::plan::Plan::NOT_RUN_EXECUTE_FUNCTION;
	}, [&](const aris::plan::PlanTarget &param)->int
	{
		param.controller->motionAtAbs(0).setTargetPos(param.count*0.002);
		return 100 - param.count;
	}, [&](aris::plan::PlanTarget &)->void
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}, "<Command name=\"tt\"/>");
	

	// make log file has enough space
	cs.planRoot().planPool().add<aris::plan::RemoveFile>("remove_file");
	cs.start();

	//for (int i = 0; i < 1000; ++i)
	//{
	//	cs.executeCmd(aris::core::Msg("en --limit_time=1"));
	//}


	try
	{
		cs.executeCmd(aris::core::Msg("rmFi --filePath=/home/kaanh/log --memo=20000"));
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}
	cs.planRoot().planPool().add<aris::plan::MoveSeries>("move_series");

	/*
	auto ec_ptr = std::make_unique<aris::control::EthercatController>();
	ec_ptr->setEsiDirs({
		std::filesystem::path("C:\\Users\\py033\\Desktop\\esi_dirs"),
		std::filesystem::path("C:\\Users\\py033\\Desktop\\esi_dirs\\Beckhoff AX5xxx")
		});

	auto str = ec_ptr->xmlString();
	std::cout << str << std::endl;
	ec_ptr->loadXmlStr(str);
	std::cout << ec_ptr->xmlString() << std::endl;

	ec_ptr->updateDeviceList();

	std::cout << ec_ptr->getDeviceList() << std::endl;
	std::cout << ec_ptr->getPdoList(0x000002E1, 0x00, 0x29001) << std::endl;
	std::cout << ec_ptr->getPdoList(0x0000009A, 0x00030924, 0x000103f4) << std::endl;
	*/

	// interaction //
	cs.interfacePool().add<aris::server::WebInterface>("", "5866", aris::core::Socket::WEB);
	cs.interfacePool().add<aris::server::WebInterface>("", "5867", aris::core::Socket::TCP);
	cs.open();
	cs.runCmdLine();
	
	return 0;
}