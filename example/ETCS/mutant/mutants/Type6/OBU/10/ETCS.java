/**
* This class is generated automatically
* {@link https://github.com/nhnghia/IF2DOT/tree/2java}
* @author nhnghia
*/
public class ETCS extends Simulator{
	public String getSystemName(){
		return "ETCS";
	}
	public static class DangerPointInfo extends DataStruct{
		public Integer distance;
		public Integer speed;
	}
	public static class OverlapInfo extends DataStruct{
		public Integer distance;
		public Integer timeout;
		public Integer distanceTimeout;
		public Integer speed;
	}
	public static class Section extends DataStruct{
		public Integer distance;
		public Integer timeout;
		public Integer timeoutStop;
	}
	public static class EndSection extends DataStruct{
		public Integer distance;
		public Integer timeout;
		public Integer timeoutStop;
		public Integer endTimeout;
		public Integer endDistance;
		public DangerPointInfo dangerInfo = new DangerPointInfo();
		public OverlapInfo overlapInfo = new OverlapInfo();
	}
	public static class MovementAuthority extends DataStruct{
		public Integer distance;
		public Integer speed;
	}
	public static enum Reason {
		DRIVER, TIME_BEFORE_EOA_LOA, TIMER_EXPIRED, TRACK_DES, TRACK_FREEUP}
	public static enum STATUS {
		NORMAL, INDICATION, OVERSPEED, WARNING, INTERVENTION}
	public final int SD = 300;
	public final int N_TRAINS = 1;
	public final int N_RBCS = 0;
	public final int MAX_SPEED = 191;
	public final int N_SECTIONS = 5;

	//Signals=============================
	public static class MA extends Signal{
		public MovementAuthority param;
		public MA(){
			source = "RBC";
			destination = "OBU";
		}
	}
	public static class MARequest extends Signal{
		public Integer param;
		public MARequest(){
			source = "OBU";
			destination = "RBC";
		}
	}
	public static class NeedSendMARequest extends Signal{
		public NeedSendMARequest(){
			source = "env";
			destination = "OBU";
		}
	}
	public static class TemporarySpeedRestrictions extends Signal{
		public TemporarySpeedRestrictions(){
			source = "env";
			destination = "OBU";
		}
	}
	public static class CurrentStatus extends Signal{
		public STATUS param;
		public CurrentStatus(){
			source = "env";
			destination = "OBU";
		}
	}
	public static class ESpeed extends Signal{
		public Integer param;
		public ESpeed(){
			source = "env";
			destination = "OBU";
		}
	}
	public static class ELocation extends Signal{
		public Integer param;
		public ELocation(){
			source = "env";
			destination = "OBU";
		}
	}
	public static class ServiceBrakingCmd extends Signal{
		public Integer param;
		public ServiceBrakingCmd(){
			source = "OBU";
			destination = "env";
		}
	}
	public static class EBcmd extends Signal{
		public Integer param;
		public EBcmd(){
			source = "OBU";
			destination = "env";
		}
	}
	public static class DMIcmd extends Signal{
		public Integer param;
		public DMIcmd(){
			source = "OBU";
			destination = "env";
		}
	}
	public static class EmergencyBrake extends Signal{
		public EmergencyBrake(){
			source = "OBU";
			destination = "env";
		}
	}
	public static class DLocation extends Signal{
		public Integer param;
		public DLocation(){
			source = "env";
			destination = "RBC";
		}
	}
	//================================
	public static class OBU extends ETCS{
		public MovementAuthority m = new MovementAuthority();
		public Integer v = 0;
		public Integer c0;
		public Integer c;
		public Integer l;
		public ETCS rbc;

		public void updateClock(int val){
			c0 = val;
			c = val;
		}

		public void state_init(){
			boolean tranFired = false;
			Signal _out;

			//transition id="tr1" dst="TEMP" input="ELocation(l)" output="output MARequest(l) to rbc"
			Integer _l = (Integer) getInput(ELocation.class, Integer.class);
			if (_l != null) {
				l = _l;
				_out = new MARequest();
				_out.param = l;
				output(_out);
				tranFired = true;
				nextState("TEMP");
			}

			//no transition can be fired, we stand at this state and try to fire again its outgoing transitions
			if (tranFired == false) noFire("init");
		}

		public void state_TEMP(){
			boolean tranFired = false;
			Signal _out;

			//transition id="tr2" dst="NORMAL" input="MA(m)" output=""
			MovementAuthority _m = (MovementAuthority) getInput(MA.class, MovementAuthority.class);
			if (_m != null) {
				m = _m;
				c = 0; //reset clock
				tranFired = true;
				nextState("NORMAL");
			}

			//no transition can be fired, we stand at this state and try to fire again its outgoing transitions
			if (tranFired == false) noFire("TEMP");
		}

		public void state_NORMAL(){
			boolean tranFired = false;
			Signal _out;

			//transition id="tr3" dst="INTERVENTION" input="" output="output EBcmd(1)"
			if ((v > m.speed)) {
				_out = new EBcmd();
				_out.param = 1;
				output(_out);
				_out = new DMIcmd();
				_out.param = m.speed;
				output(_out);
				tranFired = true;
				nextState("INTERVENTION");
			}

			//transition id="tr4" dst="INDICATION" input="" output="output DMIcmd(m.speed)"
			if ((v <= m.speed)) {
				_out = new DMIcmd();
				_out.param = m.speed;
				output(_out);
				tranFired = true;
				nextState("INDICATION");
			}

			//no transition can be fired, we stand at this state and try to fire again its outgoing transitions
			if (tranFired == false) noFire("NORMAL");
		}

		public void state_INDICATION(){
			boolean tranFired = false;
			Signal _out;

			//transition id="tr5" dst="INDICATION" input="MA(m)" output=""
			MovementAuthority _m = (MovementAuthority) getInput(MA.class, MovementAuthority.class);
			if (_m != null) {
				m = _m;
				c = 0; //reset clock
				tranFired = true;
				nextState("INDICATION");
			}

			//transition id="tr6" dst="NORMAL" input="ESpeed(v)" output=""
			Integer _v = (Integer) getInput(ESpeed.class, Integer.class);
			if (_v != null) {
				v = _v;
				c0 = 0; //reset clock
				tranFired = true;
				nextState("NORMAL");
			}

			//transition id="tr7" dst="INDICATION" input="ELocation(l)" output="output MARequest(l)"
			if (c>0) {
				Integer _l = (Integer) getInput(ELocation.class, Integer.class);
				if (_l != null) {
					l = _l;
					c = 0; //reset clock
					_out = new MARequest();
					_out.param = l;
					output(_out);
					tranFired = true;
					nextState("INDICATION");
				}
			}

			//no transition can be fired, we stand at this state and try to fire again its outgoing transitions
			if (tranFired == false) noFire("INDICATION");
		}

		public void state_INTERVENTION(){
			boolean tranFired = false;
			Signal _out;

			//transition id="tr8" dst="INDICATION" input="" output="output EBcmd(0)"
			if ((v==0)) {
				_out = new EBcmd();
				_out.param = 0;
				output(_out);
				_out = new DMIcmd();
				_out.param = m.speed;
				output(_out);
				tranFired = true;
				nextState("INDICATION");
			}

			//transition id="tr9" dst="INTERVENTION" input="ESpeed(v)" output=""
			if (c0>1) {
			if ((v > 0)) {
				Integer _v = (Integer) getInput(ESpeed.class, Integer.class);
				if (_v != null) {
					v = _v;
					c0 = 0; //reset clock
					tranFired = true;
					nextState("INTERVENTION");
				}
			}
		}
			//no transition can be fired, we stand at this state and try to fire again its outgoing transitions
			if (tranFired == false) noFire("INTERVENTION");
		}
		public String getInitStateName(){
			return "init";
		}
		public void callState(String stateName){
			if (stateName.equals("init")) state_init();
			if (stateName.equals("TEMP")) state_TEMP();
			if (stateName.equals("NORMAL")) state_NORMAL();
			if (stateName.equals("INDICATION")) state_INDICATION();
			if (stateName.equals("INTERVENTION")) state_INTERVENTION();
		}
	}
	//================================
	public static class RBC extends ETCS{
		public MovementAuthority m = new MovementAuthority();
		public Integer x;
		public Integer v;
		public Integer y;
		public Integer d;
		public Integer c;
		public Integer c1;

		public void updateClock(int val){
			c = val;
			c1 = val;
		}

		//procedure
		public Integer getReleaseSpeed(Integer d){
			
     return 80;
   
		}

		public void state_init(){
			boolean tranFired = false;
			Signal _out;

			//transition id="tr1" dst="IDLE" input="DLocation(x)" output=""
			Integer _x = (Integer) getInput(DLocation.class, Integer.class);
			if (_x != null) {
				x = _x;
				c = 0; //set clock
				c1 = 0; //set clock
				tranFired = true;
				nextState("IDLE");
			}

			//no transition can be fired, we stand at this state and try to fire again its outgoing transitions
			if (tranFired == false) noFire("init");
		}

		public void state_IDLE(){
			boolean tranFired = false;
			Signal _out;

			//transition id="tr2" dst="MA" input="MARequest(y)" output=""
			Integer _y = (Integer) getInput(MARequest.class, Integer.class);
			if (_y != null) {
				y = _y;
				tranFired = true;
				nextState("MA");
			}

			//transition id="tr3" dst="IDLE" input="DLocation(x)" output=""
			Integer _x = (Integer) getInput(DLocation.class, Integer.class);
			if (_x != null) {
				x = _x;
				c1 = 0; //reset clock
				tranFired = true;
				nextState("IDLE");
			}

			//transition id="tr4" dst="MA" input="" output=""
			if (c>=60) {
				tranFired = true;
				nextState("MA");
			}

			//no transition can be fired, we stand at this state and try to fire again its outgoing transitions
			if (tranFired == false) noFire("IDLE");
		}

		public void state_MA(){
			boolean tranFired = false;
			Signal _out;

			//transition id="tr5" dst="IDLE" input="" output="output MA(m) to obu"
			m.distance = (x - y);
			m.speed = getReleaseSpeed((x - y));
			_out = new MA();
			_out.param = m;
			output(_out);
			c = 0; //reset clock
			tranFired = true;
			nextState("IDLE");

			//no transition can be fired, we stand at this state and try to fire again its outgoing transitions
			if (tranFired == false) noFire("MA");
		}
		public String getInitStateName(){
			return "init";
		}
		public void callState(String stateName){
			if (stateName.equals("init")) state_init();
			if (stateName.equals("IDLE")) state_IDLE();
			if (stateName.equals("MA")) state_MA();
		}
	}
}