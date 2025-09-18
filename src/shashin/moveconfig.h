#ifndef MOVE_CONFIG_H
#define MOVE_CONFIG_H

namespace Alexander {
namespace MoveConfig {

// Dichiarazione come variabili thread-local
extern thread_local bool useMoveShashinLogic;
extern thread_local bool isStrategical;
extern thread_local bool isAggressive;
extern thread_local bool isFortress;

}  // namespace MoveConfig
}  // namespace Alexander

#endif  // MOVE_CONFIG_H
