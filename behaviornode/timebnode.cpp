#include "timebnode.h"

void TimerBNode::_bind_methods() {
    ObjectTypeDB::bind_method(_MD("get_delay"), &TimerBNode::get_delay);
    ObjectTypeDB::bind_method(_MD("set_delay", "delay"), &TimerBNode::set_delay);

    ObjectTypeDB::bind_method(_MD("get_time"), &TimerBNode::get_time);
    ObjectTypeDB::bind_method(_MD("time_out"), &TimerBNode::time_out);

    ObjectTypeDB::bind_method(_MD("recount"), &TimerBNode::recount);
    ObjectTypeDB::bind_method(_MD("recount_to", "time"), &TimerBNode::recount_to);

    ADD_PROPERTY( PropertyInfo( Variant::REAL, "timer/delay" ), _SCS("set_delay"),_SCS("get_delay" ) );
    BIND_VMETHOD( MethodInfo("_during_behavior", PropertyInfo(Variant::OBJECT,"target"), PropertyInfo(Variant::DICTIONARY,"env")) );
    BIND_VMETHOD( MethodInfo("_timeout_behavior", PropertyInfo(Variant::OBJECT,"target"), PropertyInfo(Variant::DICTIONARY,"env")) );
}


BehaviorNode::Status TimerBNode::_step(const Variant& target, Dictionary &env) {
    if (!env.has("timestep"))
        return STATUS_FAILURE;
    if (_time <= 0) {
	    if (!timeout) {
            _timeout_behavior(target, env);
            _script_timeout_behavior(target, env);
            timeout=true;
            return STATUS_FAILURE;
        }else {
            return BehaviorNode::_step(target, env);
        }
    }else {
        float timestep = env["timestep"];
        _time -= timestep;
        _during_behavior(target, env);
        _script_during_behavior(target, env);
        if (cancel) {
            cancel = false;
            return STATUS_FAILURE;
        }
        else {
            _traversal_children(target, env);
            return STATUS_RUNNING;
        }
    }
}

void TimerBNode::recount() {
    _time = delay;
    cancel = false;
    timeout = false;
}

void TimerBNode::_reset(const Variant &target) {
    BehaviorNode::_reset(target);
    _time=0;
    timeout = true;
}

void TimerBNode::recount_to(float t) {
    _time = t;
    cancel = false;
    timeout = false;
}

BehaviorNode::Status TimerBNode::_behavior(const Variant &target, Dictionary env) {
    _time = delay;
    cancel = false;
    timeout = false;
    return STATUS_RUNNING;
}

void TimerBNode::_script_during_behavior(const Variant &target, Dictionary &env) {
    if (get_script_instance()) {
        Variant var_env = Variant(env);
        const Variant* ptr[2]={&target,&var_env};
        get_script_instance()->call_multilevel(StringName("_during_behavior"),ptr,2);
    }
}

void TimerBNode::_script_timeout_behavior(const Variant &target, Dictionary &env) {
    if (get_script_instance()) {
        Variant var_env = Variant(env);
        const Variant* ptr[2]={&target,&var_env};
        get_script_instance()->call_multilevel(StringName("_timeout_behavior"),ptr,2);
    }
}