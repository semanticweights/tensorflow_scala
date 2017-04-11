package org.platanios.tensorflow.api

import org.platanios.tensorflow.api.ops.ArrayOps._
import org.platanios.tensorflow.api.ops.MathOps._
import org.platanios.tensorflow.api.ops.Op.createWith
import org.scalatest._

/**
  * @author Emmanouil Antonios Platanios
  */
class SessionSpec extends FlatSpec with Matchers {
  "Session run fetch by name" should "return the correct result" in {
    val graph = Graph()
    createWith(graph = graph) {
      val a = constant(Array[Array[Int]](Array[Int](2), Array[Int](3)))
      val x = placeholder(dataType = DataType.Int32, name = "X")
      subtract(constant(1), matMul(a = a, b = x, transposeA = true), name = "Y")
    }
    val session = Session(graph = graph)
    using(Tensor.create(Array[Array[Int]](Array[Int](5), Array[Int](7)))) { x =>
      val feeds = Map(graph.opOutputByName("X:0") -> x)
      val fetches = Array(graph.opOutputByName("Y:0"))
      val outputs = session.run(feeds, fetches)
      assert(outputs.size == 1)
      val expectedResult = Array[Array[Int]](Array[Int](-30))
      outputs.head.copyTo(Array.ofDim[Int](1, 1)) should equal(expectedResult)
    }
    graph.close()
  }
}
