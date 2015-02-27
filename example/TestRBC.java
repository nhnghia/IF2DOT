

import static org.junit.Assert.*;

import java.lang.reflect.InvocationTargetException;

import org.junit.Test;

public class TestRBC {

	@Test
	public void test() throws Exception {
		ETCS.OBU obu = new ETCS.OBU();
		assertTrue(obu.run("./src/obu.testcase.tc"));
	}
	
	@Test
	public void testRBC() throws Exception {
		ETCS.RBC obu = new ETCS.RBC();
		assertTrue(obu.run("./src/rbc.testcase.tc"));
	}
}
