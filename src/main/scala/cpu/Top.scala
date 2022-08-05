package cpu

import chisel3._
import chisel3.util._
import common.Consts._

class Top extends Module {
  val io = IO(new Bundle {
    val success = Output(Bool())
    val exit = Output(Bool())
  })
  val core = Module(new Core())
  val memory = Module(new Memory(Some(i => f"bootrom_${i}.hex")))
  core.io.imem <> memory.io.imem
  core.io.dmem <> memory.io.dmem
  io.success := core.io.success
  io.exit := core.io.exit
}