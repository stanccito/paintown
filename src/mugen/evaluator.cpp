#include "evaluator.h"
#include "mugen_exception.h"
#include "character.h"
#include "mugen_animation.h"
#include "ast/all.h"
#include <math.h>

/* TODO:
 * 1. Change RuntimeValue into an object that stores pointers so that the
 * overhead of copying values is not so high.
 * 2. 1 necessitates garbage collection. Implement precise mark/sweep.
 * 3. Convert interpreter into a compiler. Refresher
 *   (define intepreter (lambda (env) (lambda (input) (eval env input))))
 *   (define compiler (lambda (input) (let ([compiled (compile input)])
 *                       (lambda (env) (eval env compiled)))))
 * 4. Implement simple optimizations: constant folding, dead code elimintation.
 */

using namespace std;

namespace Mugen{

static void raise(const RuntimeValue & value, const string & expected){
    ostringstream out;
    out << "Not a " << expected << " instead was " << value.canonicalName();
    throw MugenException(out.str());
}

string toString(const RuntimeValue & value){
    if (value.isString()){
        return value.getStringValue();
    }
    raise(value, "string");
}

double toNumber(const RuntimeValue & value){
    if (value.isDouble()){
        return value.getDoubleValue();
    }
    if (value.isBool()){
        if (value.getBoolValue()){
            return 1;
        } else {
            return 0;
        }
    }
    raise(value, "number");
}

int toRangeLow(const RuntimeValue & value){
    if (value.isRange()){
        return value.getRangeLow();
    }
    raise(value, "range");
}

int toRangeHigh(const RuntimeValue & value){
    if (value.isRange()){
        return value.getRangeHigh();
    }
    raise(value, "range");
}


bool toBool(const RuntimeValue & value){
    if (value.isBool()){
        return value.getBoolValue();
    }
    if (value.isDouble()){
        return value.getDoubleValue() != 0;
    }
    raise(value, "bool");
}

/* a meta-circular evaluator! */
class Evaluator: public Ast::Walker {
public:
    Evaluator(const Environment & environment):
        environment(environment){
        }

    const Environment & environment;
    RuntimeValue result;

    /* value1 == value2 */
    RuntimeValue same(const RuntimeValue & value1, const RuntimeValue & value2){
        if (value1.type == RuntimeValue::Invalid || value2.type == RuntimeValue::Invalid){
            throw MugenException("invalid value");
        }
        switch (value1.type){
            case RuntimeValue::ListOfString : {
                switch (value2.type){
                    case RuntimeValue::String : {
                        const vector<string> & strings = value1.strings_value;
                        for (vector<string>::const_iterator it = strings.begin(); it != strings.end(); it++){
                            const string & check = *it;
                            if (check == value2.string_value){
                                return RuntimeValue(true);
                            }
                        }
                        return RuntimeValue(false);
                    }
                }
                break;
            }
            case RuntimeValue::String : {
                switch (value2.type){
                    case RuntimeValue::ListOfString : {
                        return same(value2, value1);
                    }
                    case RuntimeValue::String : {
                        return toString(value1) == toString(value2);
                    }
                }
                break;
            }
            case RuntimeValue::RangeType : {
                switch (value2.type){
                    case RuntimeValue::Double : return same(value2, value1);
                }
            }
            case RuntimeValue::Double : {
                switch (value2.type){
                    case RuntimeValue::Double : {
                        double epsilon = 0.0000001;
                        return RuntimeValue(fabs(value1.getDoubleValue() - value2.getDoubleValue()) < epsilon);
                    }
                    case RuntimeValue::RangeType : {
                        return RuntimeValue(toNumber(value1) > toRangeLow(value2) &&
                                            toNumber(value1) < toRangeHigh(value2));
                    }
                }
                break;
            }
        }

        return RuntimeValue(false);
    }
    
    RuntimeValue evalRange(const Ast::Range & range){
        int low, high;
        *range.getLow() >> low;
        *range.getHigh() >> high;
        switch (range.getRangeType()){
            case Ast::Range::AllInclusive : return RuntimeValue(low - 1, high + 1);
            case Ast::Range::AllExclusive : return RuntimeValue(low, high);
            case Ast::Range::LeftInclusiveRightExclusive : return RuntimeValue(low - 1, high);
            case Ast::Range::LeftExclusiveRightInclusive : return RuntimeValue(low, high + 1);
        }
        throw MugenException("Unexpected range type");
    }
    
    virtual void onRange(const Ast::Range & range){
        result = evalRange(range);
    }
   
    RuntimeValue evaluate(const Ast::Value * value){
        return Mugen::evaluate(value, environment);
    }

    RuntimeValue evalIdentifier(const Ast::Identifier & identifier){
        if (identifier == "command"){
            return RuntimeValue(environment.getCommands());
        }

        if (identifier == "anim"){
            return RuntimeValue(environment.getCharacter().getAnimation());
        }

        if (identifier == "alive"){
            /* FIXME */
            return RuntimeValue(true);
        }

        if (identifier == "p2statetype"){
            /* FIXME */
            return RuntimeValue(0);
        }

        if (identifier == "movecontact"){
            /* FIXME */
            return RuntimeValue(0);
        }

        if (identifier == "animtime"){
            return RuntimeValue(environment.getCharacter().getCurrentAnimation()->animationTime());
        }

        if (identifier == "palno"){
            /* FIXME */
            return RuntimeValue(1);
        }

        if (identifier == "winko"){
            /* FIXME */
            return RuntimeValue(false);
        }

        if (identifier == "movehit"){
            /* FIXME */
            return RuntimeValue(0);
        }

        if (identifier == "projhit"){
            /* FIXME */
            return RuntimeValue(0);
        }

        if (identifier == "hitshakeover"){
            return RuntimeValue(environment.getCharacter().getHitState().shakeTime <= 0);
        }

        if (identifier == "hitover"){
            return RuntimeValue(environment.getCharacter().getHitState().hitTime <= -1);
        }

        if (identifier == "canrecover"){
            return RuntimeValue(environment.getCharacter().canRecover());
        }

        if (identifier == "hitfall"){
            const HitState & state = environment.getCharacter().getHitState();
            return RuntimeValue(state.fall.fall);
        }

        /* the mugen docs don't say anything about `statetime' but its
         * most likely the same thing as `time'
         */
        if (identifier == "time" || identifier == "statetime"){
            return RuntimeValue(environment.getCharacter().getStateTime());
        }

        if (identifier == "A"){
            return RuntimeValue(string("A"));
        }
        
        if (identifier == "S"){
            /* states are just strings */
            return RuntimeValue(string("S"));
        }

        if (identifier == "C"){
            return RuntimeValue(string("C"));
        }
        
        if (identifier == "L"){
            return RuntimeValue(string("L"));
        }

        if (identifier == "statetype"){
            return RuntimeValue(environment.getCharacter().getStateType());
        }

        /* true if the player has control */
        if (identifier == "ctrl"){
            return RuntimeValue(environment.getCharacter().hasControl());
        }

        if (identifier == "stateno"){
            return RuntimeValue(environment.getCharacter().getCurrentState());
        }

        if (identifier == "power"){
            return RuntimeValue(environment.getCharacter().getPower());
        }

        if (identifier == "velocity.walk.back.x"){
            return RuntimeValue(environment.getCharacter().getWalkBackX());
        }

        if (identifier == "velocity.walk.fwd.x"){
            return RuntimeValue(environment.getCharacter().getWalkForwardX());
        }

        if (identifier == "velocity.run.fwd.x"){
            return RuntimeValue(environment.getCharacter().getRunForwardX());
        }
        
        if (identifier == "velocity.jump.neu.x"){
            return RuntimeValue(environment.getCharacter().getNeutralJumpingX());
        }
        
        if (identifier == "velocity.jump.y"){
            return RuntimeValue(environment.getCharacter().getNeutralJumpingY());
        }

        if (identifier == "prevstateno"){
            return RuntimeValue(environment.getCharacter().getPreviousState());
        }
        
        if (identifier == "velocity.run.back.x"){
            return RuntimeValue(environment.getCharacter().getRunBackX());
        }
        
        if (identifier == "velocity.run.back.y"){
            return RuntimeValue(environment.getCharacter().getRunBackY());
        }

        if (identifier == "velocity.jump.back.x"){
            return RuntimeValue(environment.getCharacter().getJumpBack());
        }

        if (identifier == "velocity.jump.fwd.x"){
            return RuntimeValue(environment.getCharacter().getJumpForward());
        }

        if (identifier == "velocity.runjump.fwd.x"){
            return RuntimeValue(environment.getCharacter().getRunJumpForward());
        }

        ostringstream out;
        out << "Unknown identifier '" << identifier.toString() << "'";
        throw MugenException(out.str());
    }

    virtual void onIdentifier(const Ast::Identifier & identifier){
        result = evalIdentifier(identifier);
    }

    RuntimeValue evalKeyword(const Ast::Keyword & keyword){
        if (keyword == "vel x"){
            return RuntimeValue(environment.getCharacter().getXVelocity());
        }

        if (keyword == "vel y"){
            return RuntimeValue(environment.getCharacter().getYVelocity());
        }
        
        if (keyword == "pos y"){
            return RuntimeValue(-environment.getCharacter().getYPosition());
        }
        
        if (keyword == "p2bodydist x"){
            /* FIXME */
            return RuntimeValue(0);
        }

        ostringstream out;
        out << "Unknown keyword '" << keyword.toString() << "'";
        throw MugenException(out.str());
    }

    virtual void onKeyword(const Ast::Keyword & keyword){
        result = evalKeyword(keyword);
    }

    RuntimeValue evalString(const Ast::String & string_value){
        string out;
        string_value >> out;
        return RuntimeValue(out);
    }

    virtual void onString(const Ast::String & string){
        result = evalString(string);
    }

    RuntimeValue evalFunction(const Ast::Function & function){
        if (function == "const"){
            return evaluate(function.getArg1());
        }

        if (function == "abs"){
            return RuntimeValue(fabs(toNumber(evaluate(function.getArg1()))));
        }

        if (function == "var"){
            int index = (int) toNumber(evaluate(function.getArg1()));
            Ast::Value * value = environment.getCharacter().getVariable(index);
            if (value == 0){
                ostringstream out;
                out << "No variable for index " << index;
                throw MugenException(out.str());
            }
            return evaluate(value);
        }

        if (function == "gethitvar"){
            const HitState & state = environment.getCharacter().getHitState();
            if (function.getArg1() == 0){
                throw MugenException("No argument given to gethitvar");
            }
            string var = function.getArg1()->toString();
            if (var == "xveladd"){
            } else if (var == "yveladd"){
            } else if (var == "type"){
            } else if (var == "animtype"){
                return RuntimeValue(state.animationType);
            } else if (var == "airtype"){
            } else if (var == "groundtype"){
                return RuntimeValue(state.groundType);
            } else if (var == "damage"){
            } else if (var == "hitcount"){
            } else if (var == "fallcount"){
            } else if (var == "hitshaketime"){
            } else if (var == "hittime"){
            } else if (var == "slidetime"){
                return RuntimeValue(state.slideTime);
            } else if (var == "ctrltime"){
            } else if (var == "recovertime"){
            } else if (var == "xoff"){
            } else if (var == "yoff"){
            } else if (var == "zoff"){
            } else if (var == "xvel"){
                return RuntimeValue(state.xVelocity);
            } else if (var == "yvel"){
                return RuntimeValue(state.yVelocity);
            } else if (var == "yaccel"){
                return RuntimeValue(state.yAcceleration);
            } else if (var == "hitid"){
            } else if (var == "chainid"){
            } else if (var == "guarded"){
            } else if (var == "fall"){
                return RuntimeValue(state.fall.fall);
            } else if (var == "fall.damage"){
            } else if (var == "fall.xvel"){
            } else if (var == "fall.yvel"){
                return RuntimeValue(state.fall.yVelocity);
            } else if (var == "fall.recover"){
            } else if (var == "fall.time"){
            } else if (var == "fall.recovertime"){
            }

            throw MugenException("Unknown gethitvar variable " + var);
        }

        if (function == "sysvar"){
            int index = (int) toNumber(evaluate(function.getArg1()));
            Ast::Value * value = environment.getCharacter().getSystemVariable(index);
            if (value == 0){
                /* non-existant variables are just false */
                return RuntimeValue(false);
                /*
                ostringstream out;
                out << "No system variable for index " << index;
                throw MugenException(out.str());
                */
            }
            return evaluate(value);
        }

        if (function == "animelem"){
            unsigned int index = (unsigned int) toNumber(evaluate(function.getArg1()));
            return RuntimeValue(environment.getCharacter().getCurrentAnimation()->getPosition() + 1 == index);
        }

        if (function == "ifelse"){
            if (toBool(evaluate(function.getArg1()))){
                return evaluate(function.getArg2());
            } else {
                return evaluate(function.getArg3());
            }
        }

        if (function == "selfanimexist"){
            int animation = (int) toNumber(evaluate(function.getArg1()));
            return RuntimeValue(environment.getCharacter().hasAnimation(animation));
        }

        /* Gets the animation-time elapsed since the start of a specified element
         * of the current animation action. Useful for synchronizing events to
         * elements of an animation action.
         *
         * (reminder: first element of an action is element 1, not 0)
         */
        if (function == "animelemtime"){
            /* FIXME */
            return RuntimeValue(0);
        }

        ostringstream out;
        out << "Unknown function '" << function.toString() << "'";
        throw MugenException(out.str());
    }

    virtual void onFunction(const Ast::Function & string){
        result = evalFunction(string);
    }

    RuntimeValue evalNumber(const Ast::Number & number){
        double x;
        number >> x;
        return RuntimeValue(x);
    }

    virtual void onNumber(const Ast::Number & number){
        result = evalNumber(number);
    }

    virtual RuntimeValue evalExpressionInfix(const Ast::ExpressionInfix & expression){
        Global::debug(1) << "Evaluate expression " << expression.toString() << endl;
        using namespace Ast;
        switch (expression.getExpressionType()){
            case ExpressionInfix::Or : {
                return RuntimeValue(toBool(evaluate(expression.getLeft())) ||
                                    toBool(evaluate(expression.getRight())));
            }
            case ExpressionInfix::XOr : {
                return RuntimeValue(toBool(evaluate(expression.getLeft())) ^
                                    toBool(evaluate(expression.getRight())));
            }
            case ExpressionInfix::And : {
                return RuntimeValue(toBool(evaluate(expression.getLeft())) &&
                                    toBool(evaluate(expression.getRight())));
            }
            case ExpressionInfix::BitwiseOr : {
                return RuntimeValue((int)toNumber(evaluate(expression.getLeft())) |
                                    (int)toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::BitwiseXOr : {
                return RuntimeValue((int)toNumber(evaluate(expression.getLeft())) ^
                                    (int)toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::BitwiseAnd : {
                return RuntimeValue((int)toNumber(evaluate(expression.getLeft())) &
                                    (int)toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::Assignment : {
                /* FIXME: is this needed? */
                break;
            }
            case ExpressionInfix::Equals : {
                return same(evaluate(expression.getLeft()), evaluate(expression.getRight()));
                break;
            }
            case ExpressionInfix::Unequals : {
                return RuntimeValue(!toBool(same(evaluate(expression.getLeft()), evaluate(expression.getRight()))));
            }
            case ExpressionInfix::GreaterThanEquals : {
                return RuntimeValue(toNumber(evaluate(expression.getLeft())) >= toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::GreaterThan : {
                return RuntimeValue(toNumber(evaluate(expression.getLeft())) > toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::LessThanEquals : {
                return RuntimeValue(toNumber(evaluate(expression.getLeft())) <= toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::LessThan : {
                return RuntimeValue(toNumber(evaluate(expression.getLeft())) < toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::Add : {
                return RuntimeValue(toNumber(evaluate(expression.getLeft())) + toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::Subtract : {
                return RuntimeValue(toNumber(evaluate(expression.getLeft())) - toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::Multiply : {
                return RuntimeValue(toNumber(evaluate(expression.getLeft())) * toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::Divide : {
                /* FIXME: catch divide by 0 */
                return RuntimeValue(toNumber(evaluate(expression.getLeft())) / toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::Modulo : {
                return RuntimeValue((int)toNumber(evaluate(expression.getLeft())) % (int)toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::Power : {
                return RuntimeValue(pow(toNumber(evaluate(expression.getLeft())), toNumber(evaluate(expression.getRight()))));
            }
        }

        ostringstream out;
        out << "Unknown expression: " << expression.toString();
        throw MugenException(out.str());
    }

    virtual void onExpressionInfix(const Ast::ExpressionInfix & expression){
        result = evalExpressionInfix(expression);
    }

    RuntimeValue evalExpressionUnary(const Ast::ExpressionUnary & expression){
        switch (expression.getExpressionType()){
            case Ast::ExpressionUnary::Not : {
                return RuntimeValue(!toBool(evaluate(expression.getExpression())));
            }
            case Ast::ExpressionUnary::Minus : {
                return RuntimeValue(-toNumber(evaluate(expression.getExpression())));
            }
            case Ast::ExpressionUnary::Negation : {
                return RuntimeValue(~(int)toNumber(evaluate(expression.getExpression())));
            }
        }

        ostringstream out;
        out << "Unknown expression: " << expression.toString();
        throw MugenException(out.str());
    }
    
    virtual void onExpressionUnary(const Ast::ExpressionUnary & expression){
        result = evalExpressionUnary(expression);
    }
};

RuntimeValue evaluate(const Ast::Value * value, const Environment & environment){
    try{
        Evaluator eval(environment);
        value->walk(eval);
        return eval.result;
    } catch (const MugenException & e){
        ostringstream out;
        out << "Error while evaluating expression `" << value->toString() << "': " << e.getReason();
        throw MugenException(out.str());
    }
}

}
