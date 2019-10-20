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
            this.registers = new WindowsFormsApp1.RegistersControl();
            this.nesDisplay = new NesEmu.NesDisplayControl();
            this.dissassembly = new NesEmu.DisassemblyControl();
            this.SuspendLayout();
            // 
            // registers
            // 
            this.registers.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.registers.Cpu = null;
            this.registers.Location = new System.Drawing.Point(1566, 19);
            this.registers.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.registers.Name = "registers";
            this.registers.Size = new System.Drawing.Size(768, 314);
            this.registers.TabIndex = 1;
            this.registers.Text = "registers";
            // 
            // nesDisplay
            // 
            this.nesDisplay.Location = new System.Drawing.Point(20, 19);
            this.nesDisplay.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.nesDisplay.Name = "nesDisplay";
            this.nesDisplay.Size = new System.Drawing.Size(1536, 1421);
            this.nesDisplay.System = null;
            this.nesDisplay.TabIndex = 0;
            this.nesDisplay.Text = "nesDisplayControl";
            // 
            // dissassembly
            // 
            this.dissassembly.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.dissassembly.Bus = null;
            this.dissassembly.InstructionCount = 32;
            this.dissassembly.Location = new System.Drawing.Point(1566, 341);
            this.dissassembly.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.dissassembly.Name = "dissassembly";
            this.dissassembly.ProgramCounter = ((ushort)(0));
            this.dissassembly.Size = new System.Drawing.Size(768, 1099);
            this.dissassembly.StartAddress = ((ushort)(0));
            this.dissassembly.TabIndex = 2;
            this.dissassembly.Text = "disassemblyControl1";
            // 
            // EmulatorForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(18F, 37F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(2352, 1455);
            this.Controls.Add(this.dissassembly);
            this.Controls.Add(this.registers);
            this.Controls.Add(this.nesDisplay);
            this.Font = new System.Drawing.Font("Consolas", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.Name = "EmulatorForm";
            this.Text = "NesEmu";
            this.ResumeLayout(false);

        }

        #endregion

        private NesDisplayControl nesDisplay;
        private WindowsFormsApp1.RegistersControl registers;
        private DisassemblyControl dissassembly;
    }
}

