

import static org.junit.Assert.*;

import java.lang.reflect.InvocationTargetException;

import org.junit.Test;

public class TestRBC {

	@Test
	public void test() throws Exception {
		ETCS.OBU obu = new ETCS.OBU();
		assertTrue(obu.run("./src/testcase.tc"));
	}
}
