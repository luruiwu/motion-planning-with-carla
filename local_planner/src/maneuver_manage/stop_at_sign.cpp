#include "maneuver_manage/stop_at_sign.hpp"
namespace planning {

bool StopAtSignState::Enter(ManeuverPlanner *manuever_planner) {

}
void StopAtSignState::Exit(ManeuverPlanner *manuvever_planner) {

}
bool StopAtSignState::Execute(ManeuverPlanner *manuever_planner) {

  return false;
}
State &StopAtSignState::Instance() {
  static StopAtSignState instance;
  return instance;
}
}