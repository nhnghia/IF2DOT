
import java.io.*;
import java.lang.reflect.Field;
import java.util.*;

/**
 * {@link https://github.com/nhnghia/IF2DOT/tree/2java}
 * @author nhnghia
 *
 */
public class Simulator {
	
	//Data type definition
	public static abstract class Event{}
	public static abstract class Signal extends Event{
		/**
		 * source and destination defined in model
		 */
		public String source;
		public String destination;
		public Object param;
		public boolean isInput;
		
		public String toString(){
			String str = "!";
			if (isInput)
				str = "?";
			str = String.format("%s	%s	%s{%s}	%s", str, this.source, this.getClass().getSimpleName(), param, destination);
			return str;
		}
		
		public boolean isLike(Class<?> cls){
			if (! this.getClass().equals(cls))
				return false;
			return true;
		}
	}
	
	public static class Delay extends Event{
		public int delay;
		public String toString(){
			return "delay	" + delay;
		}
	}
	
	public static class DataStruct{
		public String toString(){
			Field[] fields = this.getClass().getFields();
			String str = "";
			for (int i=0; i<fields.length; i++)
				try {
					if (i == 0)
						str = fields[0].get(this).toString();
					else
						str += "," + fields[i].get(this);
				} catch (Exception e) {}
			return "{" + str + "}";
		}
	}
	
	//methods to be overridden in generated class
	public void updateClock(int val){}
	protected void callState(String stateName){}
	public void state_init(){}
	/**
	 * to be overridden by extended class 
	 */
	public String getSystemName(){
		return "Simulator";
	}
	
	/**
	 * 
	 */
	public String getInitStateName(){
		return "init";
	}
	
	
	int stateCallNumber;
	int currentEventIndex;
	List<Event> eventList;
	Set<String> expectedSignalsAtState;	//when no outgoing trans of a state can be fired, this list contains its expected in/output signals
	boolean verdict;
	boolean hasActivity;
	String lastStateName;
	
	/**
	 * This method is called when no outgoing transition of state {@code stateName} can be fired
	 * @param stateName
	 */
	protected void noFire(String stateName){
		verdict = false;
		
		System.out.println(" - End at state: " + stateName);
		System.out.print(" - Simulator expects input/outputs: ");
		
		Iterator<String> i = expectedSignalsAtState.iterator();
		while (i.hasNext()){
			String name = (String) i.next();
		    System.out.print(name + ",   ");
		}
		System.out.println();
		
		Event ev = getCurrentSignal();
		if (ev != null)
			System.out.println(" - Expected event in test case: " + ev);
			
	}
	
	protected void nextState(String stateName){
		
		if (verdict == false){
			return;
		}
		
		//all events in test case are executed 
		if (currentEventIndex >= eventList.size()){
			System.out.println("All events in the test case are executed successfully");
			return;
		}
		
		//continue executing only if input/delay/output conform to test case
		System.out.println("====" + stateName + "================" + lastStateName);
				
		Event ev = eventList.get(currentEventIndex);
		if (ev instanceof Delay){
			Delay delay = (Delay) ev;
			updateClock(delay.delay);
		}
		
		//the list is reset at begining each state;
		expectedSignalsAtState = null;
		expectedSignalsAtState = new HashSet<String>();
		
		//when a state is called two times consecutivelly and there is no input/output/time events 
		if ((!hasActivity && lastStateName.equals(stateName)) || stateCallNumber > 20){
			System.out.println("loop forever");
			verdict = false;
			return;
		}
		
		stateCallNumber ++;
		
		hasActivity = false;
		lastStateName = stateName;
		
		callState(stateName);
	}
	/**
	 * get current input
	 * @param signalClass
	 * @param objClass
	 * @return NULL if current input is not instanceof signalClass
	 */
	protected Object getInput(Class<?> signalClass, Class<?> objClass){

		try{
		expectedSignalsAtState.add("?" + signalClass.getSimpleName());
		
		if (currentEventIndex > eventList.size()){
			verdict = true;
			return null;
		}
		
		Signal ev = getCurrentSignal();
		if (ev.isInput == false){
			return null;
		}
			
		if (ev.isLike(signalClass) == true){
			System.out.println(ev);
			currentEventIndex += 1;
			
			hasActivity = true;
			
			return ((Signal)ev).param;
		}
	}catch( Exception ex){}
		return null;
	}
	
	protected void output(Signal output){
		System.out.println("----");
		Signal ev = getCurrentSignal();
		
		if (ev == null && ev.toString().equals(output.toString())){
			System.out.println(ev);
			currentEventIndex += 1;
			hasActivity = true;
			return;
		}
		
		System.out.println(" - Fail output:"
				+ "\n	+ Simulator output:   " + output
				+ "\n	+ Test case expectes: " + ev);
		verdict = false;
	}
	
	/**
	 * Run simulator on the list of events
	 * @param testCase path to file containing test case
	 * @return return true (pass) if<br/>
	 *  + all inputs and delays of {@code lst} are executed<br/>
	 *  + and all outputs of {@code lst} and the ones of simulator are the same <br/>
	 * otherwise return false (fail)
	 * @throws Exception 
	 */
	public boolean run(String testCase) throws Exception{
		stateCallNumber = 0;
		currentEventIndex = 0;
		eventList = getTestCases(testCase);
		verdict = true;
		
		hasActivity = false;
		lastStateName = "";
		nextState(getInitStateName());// --> nextState() --> callState()	--> state_...()
		
		//system was terminated before executing all test case
		if (currentEventIndex < eventList.size()){
			System.out.println("System was terminated before executing all events in the test case");
			verdict = false;
		}
		
		return verdict;
	}
	
	
	/**
	 * Exception when parser a test case generated by TestGen-IFx ({@link https://github.com/nhnghia/TestGen-IFx})
	 * @author nhnghia
	 *
	 */
	public static class TestCaseParserException extends Exception{
		private static final long serialVersionUID = 1L;
		public String line;
		public TestCaseParserException(String l){
			line = l;
		}
	}
	
	
	/**
	 * get event list from a test case file
	 * @param path
	 * @return
	 * @throws TestCaseParserException
	 * @throws IOException 
	 * @throws IllegalAccessException 
	 * @throws InstantiationException 
	 * @throws ClassNotFoundException 
	 */
	public List<Event> getTestCases(String path) throws TestCaseParserException, IOException, ClassNotFoundException, InstantiationException, IllegalAccessException{
		List<Event> lst = new ArrayList<Event>();
		
		BufferedReader br = new BufferedReader(new FileReader(path));
		String line;
		while ((line = br.readLine()) != null) {
				lst.add(parserTestCase(line));
		}
		br.close();
		
		return lst;
	}
	
	/**
	 * Parser an event from a string
	 * @param str, e.g., "!;	t;	 ELocation{0};	{OBU}0"
	 * @return
	 * @throws ClassNotFoundException 
	 * @throws SecurityException 
	 * @throws IllegalAccessException 
	 */
	public Event parserTestCase(String str) throws ClassNotFoundException, InstantiationException, IllegalAccessException, TestCaseParserException{
		if (str.startsWith("delay")){
			Delay delay = new Delay();
			delay.delay = Integer.parseInt(str.split(" ", -1)[1]);
			return delay;
		}
		String[] s = str.split(";\t", -1);
		String type = s[0],
				src = s[1], 
				dst = s[3],
				className = s[2].split("\\{")[0].trim(),
				data = s[2].substring(className.length());
		
		Class<?> cls = Class.forName(getSystemName() + "$" + className);
		Signal ev = (Signal) cls.newInstance();
		ev.isInput = type.equals("?");
		
		//inverse event direction when it from/to tester t
		if (src.equals("t") || dst.equals("t")){
			ev.isInput = ! ev.isInput;
		}
		
		if (src.equals("t"))
			src = "env";
		if (dst.equals("t"))
			dst = "env";
		
		ev.source = getContentInside(src);
		ev.destination = getContentInside(dst);
		
		//create param
		Field fields[] = cls.getFields();
		for (int i=0; i<fields.length; i++){
			if (fields[i].getName().equals("param")){
				ev.param = getValue(fields[i].getType(), data);
				break;
			}
		}
			
		return ev;
	}
	
	
	/**
	 * Create an object with type of {@code type} with value {@code data}
	 * @param type
	 * @param data
	 * @return
	 * @throws TestCaseParserException
	 * @throws IllegalAccessException 
	 * @throws InstantiationException 
	 */
	private Object getValue(Class<?> type, String data) throws TestCaseParserException, InstantiationException, IllegalAccessException{
		data = getContentInside(data);
		if (type.getSimpleName().equals("Integer")){
			Integer obj = Integer.parseInt(data);
			return obj;
		}
		else if (type.getSimpleName().equals("String")){
			return data;
		}
		
		//MA{{0,80}}
		//data = {0,80}
		if (! data.startsWith("{"))
			throw new TestCaseParserException(data);
		
		
		data = getContentInside(data);
		String [] str = data.split(",");
		Object obj = type.newInstance();
		
		Field fields[] = type.getFields();
		if (fields.length != str.length)
			throw new TestCaseParserException(data);
		
		for (int i=0; i<fields.length; i++){
				fields[i].set(obj, getValue(fields[i].getType(), str[i]));;
		}
		
		return obj;
	}
	/**
	 * 
	 * @param str: {OBU}0
	 * @return "OBU"
	 */
	private String getContentInside(String str){
		if (str.indexOf("{") == -1)
			return str;
		
		str = str.substring(str.indexOf("{"));
		//find the } corresponding
		int d = 0, index = 0;
		for (index=0; index <str.length(); index ++){
			switch (str.charAt(index)) {
				case '{' : d ++;
							break;
				case '}' : d --;
							break;
			}
			if (d <= 0)
				break;
		}
		str = str.substring(1, index);
		return str;
	}
	
	Signal getCurrentSignal(){
		if (currentEventIndex >= eventList.size())
			return null;
		Event ev = eventList.get(currentEventIndex);
		
		//update clocks of simulator when Delay
		while (ev instanceof Delay){
			Delay d = (Delay) ev;
			System.out.println(d);
			updateClock(d.delay);
			ev = eventList.get(++ currentEventIndex);
		}
		return (Signal) ev;
	}
	
}
