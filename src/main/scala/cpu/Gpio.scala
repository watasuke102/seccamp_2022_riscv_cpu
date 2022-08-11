package cpu

import chisel3._
import chisel3.util._
import common.Consts._
import chisel3.stage.ChiselStage

class Gpio() extends Module {
  val io = IO(new Bundle {
    val mem = new DmemPortIo
    val out = Output(UInt(32.W))
    val lcd_out = Output(UInt(32.W))
  })

  val out = RegInit(0.U(32.W))
  val lcd_out = RegInit(0.U(32.W))
  // lcd_out := io.mem.addr(10, 2)
  
  io.out := out
  io.lcd_out := lcd_out
  
  io.mem.rdata := "xdeadbeef".U
  io.mem.rvalid := true.B
  io.mem.rdata := MuxLookup(io.mem.addr(10, 2), "xDEADBEEF".U, Seq(
      0.U -> out, // Output
      8.U -> lcd_out,
  ))
  when(io.mem.wen) {
      val mask = Cat((0 to 3).map(i => Mux(io.mem.wstrb(i), 0xff.U(8.W), 0x00.U(8.W))).reverse)
      switch(io.mem.addr(10,2)) {
        // Output
        is(0.U) {
          out := (out & ~mask) | (io.mem.wdata & mask)
        }
        is(8.U) {
          lcd_out := io.mem.wdata
        }
    }
  }
}