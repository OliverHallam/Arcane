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
            this.nesDisplay = new NesEmu.NesDisplayControl();
            this.SuspendLayout();
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
            this.ClientSize = new System.Drawing.Size(1048, 983);
            this.Controls.Add(this.nesDisplay);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Name = "EmulatorForm";
            this.Text = "NesEmu";
            this.ResumeLayout(false);

        }

        #endregion

        private NesDisplayControl nesDisplay;
    }
}

