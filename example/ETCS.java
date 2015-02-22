/**
* This class is generated automatically
* {@link https://github.com/nhnghia/IF2DOT/tree/2java}
* @author nhnghia
*/
public class ETCS extends Simulator{
	public String getSystemName(){
		return "ETCS";
	}
	public static class DangerPointInfo{
		public Integer distance;
		public Integer speed;
		public String toString(){
			return distance + "," + speed;
		}
	}
	public static class OverlapInfo{
		public Integer distance;
		public Integer timeout;
		public Integer distanceTimeout;
		public Integer speed;
		public String toString(){
			return distance + "," + timeout + "," + distanceTimeout + "," + speed;
		}
	}
	public static class Section{
		public Integer distance;
		public Integer timeout;
		public Integer timeoutStop;
		public String toString(){
			return distance + "," + timeout + "," + timeoutStop;
		}
	}
	public static class EndSection{
		public Integer distance;
		public Integer timeout;
		public Integer timeoutStop;
		public Integer endTimeout;
		public Integer endDistance;
		public DangerPointInfo dangerInfo;
		public OverlapInfo overlapInfo;
		public String toString(){
			return distance + "," + timeout + "," + timeoutStop + "," + endTimeout + "," + endDistance + "," + dangerInfo + "," + overlapInfo;
		}
	}
	public static class MovementAuthority{
		public Integer distance;
		public Integer speed;
		public String toString(){
			return distance + "," + speed;
		}
	}
	public static enum Reason {
		DRIVER, TIME_BEFORE_EOA_LOA, TIMER_EXPIRED, TRACK_DES, TRACK_FREEUP}
	public static enum STATUS {
		NORMAL, INDICATION, OVERSPEED, WARNING, INTERVENTION}
	public final int SD = 300;
	public final int N_TRAINS = 1;
	public final int N_RBCS = 0;
	public final int MAX_SPEED = 190;
	public final int N_SECTIONS = 1;
	public Integer d;

	//Signals=============================
	public static class MA extends Signal{
		public static String source = "RBC";
		public static String destination = "OBU";
		public MovementAuthority param;
	}
	public static class MARequest extends Signal{
		public static String source = "OBU";
		public static String destination = "RBC";
		public Integer param;
	}
	public static class NeedSendMARequest extends Signal{
		public static String source = "env";
		public static String destination = "OBU";
	}
	public static class TemporarySpeedRestrictions extends Signal{
		public static String source = "env";
		public static String destination = "OBU";
	}
	public static class CurrentStatus extends Signal{
		public static String source = "env";
		public static String destination = "OBU";
		public STATUS param;
	}
	public static class ESpeed extends Signal{
		public static String source = "env";
		public static String destination = "OBU";
		public Integer param;
	}
	public static class ELocation extends Signal{
		public static String source = "env";
		public static String destination = "OBU";
		public Integer param;
	}
	public static class ServiceBrakingCmd extends Signal{
		public static String source = "OBU";
		public static String destination = "env";
		public Integer param;
	}
	public static class EBcmd extends Signal{
		public static String source = "OBU";
		public static String destination = "env";
		public Integer param;
	}
	public static class DMIcmd extends Signal{
		public static String source = "OBU";
		public static String destination = "env";
		public Integer param;
	}
	public static class EmergencyBrake extends Signal{
		public static String source = "OBU";
		public static String destination = "env";
	}
	public static class DLocation extends Signal{
		public static String source = "env";
		public static String destination = "RBC";
		public Integer param;
	}
	//================================
	public static class OBU extends ETCS{
		public MovementAuthority m;
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

			//transition id="tr1" dst="TEMP" input="ELocation(l)" output="output MARequest(l) to rbc"
			Integer _l = (Integer) getInput(ELocation.class, Integer.class);
			if (_l != null) {
				l = _l;
				output(MARequest.class, l);
				tranFired = true;
				nextState("TEMP");
			}

			//no transition can be fired, we stand at this state and try to fire again its outgoing transitions
			if (tranFired == false) noFire("init");
		}

		public void state_TEMP(){
			boolean tranFired = false;

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

			//transition id="tr3" dst="INTERVENTION" input="" output="output EBcmd(1)"
			if ((v > m.speed)) {
				output(EBcmd.class, 1);
				output(DMIcmd.class, m.speed);
				tranFired = true;
				nextState("INTERVENTION");
			}

			//transition id="tr4" dst="INDICATION" input="" output="output DMIcmd(m.speed)"
			if ((v <= m.speed)) {
				output(DMIcmd.class, m.speed);
				tranFired = true;
				nextState("INDICATION");
			}

			//no transition can be fired, we stand at this state and try to fire again its outgoing transitions
			if (tranFired == false) noFire("NORMAL");
		}

		public void state_INDICATION(){
			boolean tranFired = false;

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
			if (c>60) {
				Integer _l = (Integer) getInput(ELocation.class, Integer.class);
				if (_l != null) {
					l = _l;
					c = 0; //reset clock
					output(MARequest.class, l);
					tranFired = true;
					nextState("INDICATION");
				}
			}

			//no transition can be fired, we stand at this state and try to fire again its outgoing transitions
			if (tranFired == false) noFire("INDICATION");
		}

		public void state_INTERVENTION(){
			boolean tranFired = false;

			//transition id="tr8" dst="INDICATION" input="" output="output EBcmd(0)"
			if ((v==0)) {
				output(EBcmd.class, 0);
				output(DMIcmd.class, m.speed);
				tranFired = true;
				nextState("INDICATION");
			}

			//transition id="tr9" dst="INTERVENTION" input="ESpeed(v)" output=""
			if ((v > 0)) {
				Integer _v = (Integer) getInput(ESpeed.class, Integer.class);
				if (_v != null) {
					v = _v;
					c0 = 0; //reset clock
					tranFired = true;
					nextState("INTERVENTION");
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
		public MovementAuthority m;
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
			
     return 80;//rand() % 5;
   
		}

		public void state_init(){
			boolean tranFired = false;

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

			//transition id="tr2" dst="MA" input="MARequest(y)" output=""
			if (c>1) {
				Integer _y = (Integer) getInput(MARequest.class, Integer.class);
				if (_y != null) {
					y = _y;
					tranFired = true;
					nextState("MA");
				}
			}

			//transition id="tr3" dst="IDLE" input="DLocation(x)" output=""
			if (c1>40) {
				Integer _x = (Integer) getInput(DLocation.class, Integer.class);
				if (_x != null) {
					x = _x;
					c1 = 0; //reset clock
					tranFired = true;
					nextState("IDLE");
				}
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

			//transition id="tr5" dst="IDLE" input="" output="output MA(m) to obu"
			m.distance = (x - y);
			m.speed = getReleaseSpeed((x - y));
			output(MA.class, m);
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