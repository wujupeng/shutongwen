using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;
using System.Collections.ObjectModel;

namespace ShuTongWen.UI
{
    public sealed partial class MainWindow : Window
    {
        private ObservableCollection<CandidateItem> Candidates { get; set; }

        public MainWindow()
        {
            this.InitializeComponent();
            Candidates = new ObservableCollection<CandidateItem>();
            CandidateList.ItemsSource = Candidates;
            InitializeWindow();
        }

        private void InitializeWindow()
        {
            this.SetWindowStyle();
        }

        private void SetWindowStyle()
        {
            var presenter = this.AppWindow.Presenter as Microsoft.UI.Windowing.OverlappedPresenter;
            if (presenter != null)
            {
                presenter.SetBorderAndTitleBar(false, false);
            }
        }

        public void UpdateComposition(string text)
        {
            CompositionText.Text = text;
        }

        public void UpdateCandidates(IEnumerable<CandidateItem> candidates)
        {
            Candidates.Clear();
            foreach (var candidate in candidates)
            {
                Candidates.Add(candidate);
            }
        }

        public void ShowAt(int x, int y)
        {
            this.AppWindow.MoveAndResize(new Windows.Graphics.RectInt32(x, y, 400, 300));
            this.Activate();
        }

        public void HideWindow()
        {
            this.Hide();
        }

        private void CandidateList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0)
            {
                var selected = e.AddedItems[0] as CandidateItem;
                if (selected != null)
                {
                    OnCandidateSelected(selected);
                }
            }
        }

        private void OnCandidateSelected(CandidateItem item)
        {
            CandidateSelected?.Invoke(this, new CandidateSelectedEventArgs(item));
        }

        public event EventHandler<CandidateSelectedEventArgs> CandidateSelected;
    }

    public class CandidateItem
    {
        public int Index { get; set; }
        public string Text { get; set; }
        public string Pinyin { get; set; }
        public int Frequency { get; set; }
    }

    public class CandidateSelectedEventArgs : EventArgs
    {
        public CandidateItem Item { get; }

        public CandidateSelectedEventArgs(CandidateItem item)
        {
            Item = item;
        }
    }
}