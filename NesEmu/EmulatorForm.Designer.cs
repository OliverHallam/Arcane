namespace NesEmu
{
    partial class EmulatorForm
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.dissassembly = new NesEmu.DisassemblyControl();
            this.registers = new NesEmu.RegistersControl();
            this.nesDisplay = new NesEmu.NesDisplayControl();
            this.patternLeft = new NesEmu.PatternTableControl();
            this.patternRight = new NesEmu.PatternTableControl();
            this.memory = new NesEmu.MemoryControl();
            this.SuspendLayout();
            // 
            // dissassembly
            // 
            this.dissassembly.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.dissassembly.Bus = null;
            this.dissassembly.InstructionCount = 24;
            this.dissassembly.Location = new System.Drawing.Point(1062, 343);
            this.dissassembly.Margin = new System.Windows.Forms.Padding(4);
            this.dissassembly.Name = "dissassembly";
            this.dissassembly.ProgramCounter = ((ushort)(0));
            this.dissassembly.Size = new System.Drawing.Size(632, 1156);
            this.dissassembly.StartAddress = ((ushort)(0));
            this.dissassembly.TabIndex = 2;
            this.dissassembly.Text = "disassemblyControl1";
            // 
            // registers
            // 
            this.registers.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.registers.Location = new System.Drawing.Point(1062, 19);
            this.registers.Margin = new System.Windows.Forms.Padding(4);
            this.registers.Name = "registers";
            this.registers.Size = new System.Drawing.Size(632, 314);
            this.registers.System = null;
            this.registers.TabIndex = 1;
            this.registers.Text = "registers";
            // 
            // nesDisplay
            // 
            this.nesDisplay.Location = new System.Drawing.Point(20, 19);
            this.nesDisplay.Margin = new System.Windows.Forms.Padding(4);
            this.nesDisplay.Name = "nesDisplay";
            this.nesDisplay.Size = new System.Drawing.Size(1024, 960);
            this.nesDisplay.System = null;
            this.nesDisplay.TabIndex = 0;
            this.nesDisplay.Text = "nesDisplayControl";
            // 
            // patternLeft
            // 
            this.patternLeft.Location = new System.Drawing.Point(20, 987);
            this.patternLeft.Name = "patternLeft";
            this.patternLeft.Size = new System.Drawing.Size(512, 512);
            this.patternLeft.System = null;
            this.patternLeft.TabIndex = 3;
            this.patternLeft.TableAddress = ((ushort)(0));
            this.patternLeft.Text = "patternTableControl1";
            // 
            // patternRight
            // 
            this.patternRight.Location = new System.Drawing.Point(532, 987);
            this.patternRight.Name = "patternRight";
            this.patternRight.Size = new System.Drawing.Size(512, 512);
            this.patternRight.System = null;
            this.patternRight.TabIndex = 4;
            this.patternRight.TableAddress = ((ushort)(0));
            this.patternRight.Text = "patternTableControl2";
            // 
            // memory
            // 
            this.memory.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.memory.Bus = null;
            this.memory.Location = new System.Drawing.Point(1702, 19);
            this.memory.Name = "memory";
            this.memory.Size = new System.Drawing.Size(1116, 1480);
            this.memory.TabIndex = 5;
            this.memory.Text = "memoryControl1";
            // 
            // EmulatorForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(18F, 37F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(2830, 1511);
            this.Controls.Add(this.memory);
            this.Controls.Add(this.patternRight);
            this.Controls.Add(this.patternLeft);
            this.Controls.Add(this.dissassembly);
            this.Controls.Add(this.registers);
            this.Controls.Add(this.nesDisplay);
            this.Font = new System.Drawing.Font("Consolas", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "EmulatorForm";
            this.Text = "NesEmu";
            this.ResumeLayout(false);

        }

        #endregion

        private NesDisplayControl nesDisplay;
        private RegistersControl registers;
        private DisassemblyControl dissassembly;
        private PatternTableControl patternLeft;
        private PatternTableControl patternRight;
        private MemoryControl memory;
    }
}

