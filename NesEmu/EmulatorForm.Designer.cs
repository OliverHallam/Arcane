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
            this.SuspendLayout();
            // 
            // registers
            // 
            this.registers.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.registers.Cpu = null;
            this.registers.Font = new System.Drawing.Font("Consolas", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.registers.Location = new System.Drawing.Point(1044, 13);
            this.registers.Name = "registers";
            this.registers.Size = new System.Drawing.Size(512, 307);
            this.registers.TabIndex = 1;
            this.registers.Text = "registers";
            // 
            // nesDisplay
            // 
            this.nesDisplay.Location = new System.Drawing.Point(13, 13);
            this.nesDisplay.Name = "nesDisplay";
            this.nesDisplay.Size = new System.Drawing.Size(1024, 960);
            this.nesDisplay.System = null;
            this.nesDisplay.TabIndex = 0;
            this.nesDisplay.Text = "nesDisplayControl";
            // 
            // EmulatorForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(12F, 25F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1568, 983);
            this.Controls.Add(this.registers);
            this.Controls.Add(this.nesDisplay);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Name = "EmulatorForm";
            this.Text = "NesEmu";
            this.ResumeLayout(false);

        }

        #endregion

        private NesDisplayControl nesDisplay;
        private WindowsFormsApp1.RegistersControl registers;
    }
}

