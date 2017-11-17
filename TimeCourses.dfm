object Graphs: TGraphs
  Left = 2
  Top = 290
  Caption = #1043#1088#1072#1092#1080#1082#1080
  ClientHeight = 643
  ClientWidth = 522
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  Position = poDesigned
  OnHide = OnClosGraphs
  OnResize = Resized
  PixelsPerInch = 96
  TextHeight = 13
  object SignalTrack: TChart
    Left = 8
    Top = 8
    Width = 223
    Height = 174
    AllowPanning = pmNone
    BackWall.Brush.Color = clWhite
    BackWall.Brush.Style = bsClear
    Title.Alignment = taLeftJustify
    Title.Text.Strings = (
      '')
    BottomAxis.Automatic = False
    BottomAxis.AutomaticMaximum = False
    BottomAxis.AutomaticMinimum = False
    BottomAxis.AxisValuesFormat = '#,##0.#'
    BottomAxis.ExactDateTime = False
    BottomAxis.Increment = 1.000000000000000000
    BottomAxis.LabelStyle = talValue
    BottomAxis.Maximum = 25.000000000000000000
    BottomAxis.Title.Caption = #1074#1088#1077#1084#1103', '#1084#1089
    DepthAxis.Automatic = False
    DepthAxis.AutomaticMaximum = False
    DepthAxis.AutomaticMinimum = False
    DepthAxis.Labels = False
    DepthAxis.Maximum = 1.130000000000000000
    DepthAxis.Minimum = 0.130000000000000300
    LeftAxis.AxisValuesFormat = '#,##0.#'
    LeftAxis.ExactDateTime = False
    LeftAxis.Increment = 0.100000000000000000
    LeftAxis.LabelsMultiLine = True
    LeftAxis.Title.Caption = #1085#1072#1087#1088#1103#1078#1077#1085#1080#1077', '#1084#1042
    Legend.Visible = False
    RightAxis.Automatic = False
    RightAxis.AutomaticMaximum = False
    RightAxis.AutomaticMinimum = False
    RightAxis.Labels = False
    RightAxis.Visible = False
    TopAxis.Automatic = False
    TopAxis.AutomaticMaximum = False
    TopAxis.AutomaticMinimum = False
    TopAxis.Labels = False
    TopAxis.Visible = False
    View3D = False
    View3DOptions.Elevation = 316
    View3DOptions.Perspective = 21
    TabOrder = 0
    OnDblClick = GraphChartDblClick
    OnMouseDown = SignalTrackMouseDown
    object Label1: TLabel
      Left = 45
      Top = 7
      Width = 54
      Height = 13
      Caption = #1089#1080#1075#1085#1072#1083', '#8470
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clBlue
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label2: TLabel
      Left = 191
      Top = 7
      Width = 11
      Height = 13
      Caption = #1080#1079
      Visible = False
    end
    object CrntSig: TEdit
      Left = 103
      Top = 4
      Width = 40
      Height = 21
      BiDiMode = bdRightToLeft
      MaxLength = 10
      ParentBiDiMode = False
      ReadOnly = True
      TabOrder = 0
      Text = '0'
      OnKeyDown = CrntSigKeyDown
      OnKeyPress = CrntSigKeyPress
      OnKeyUp = CrntSigKeyUp
    end
    object sigNumUpDwn: TUpDown
      Left = 145
      Top = 4
      Width = 40
      Height = 21
      Enabled = False
      Min = 1
      Max = 1
      Orientation = udHorizontal
      Position = 1
      TabOrder = 1
      Wrap = True
      OnChangingEx = sigNumUpDwnChangingEx
    end
    object csSpecPoint: TPointSeries
      Marks.ArrowLength = 0
      Marks.BackColor = clSilver
      Marks.Visible = False
      SeriesColor = clGreen
      Title = 'csSpecPoint'
      Pointer.Brush.Color = clBlack
      Pointer.InflateMargins = True
      Pointer.Style = psCircle
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
  end
  object AmplInT: TChart
    Tag = 1
    Left = 171
    Top = 59
    Width = 165
    Height = 127
    AllowPanning = pmNone
    BackWall.Brush.Color = clWhite
    BackWall.Brush.Style = bsClear
    Title.Text.Strings = (
      #1040#1084#1087#1083#1080#1090#1091#1076#1072)
    OnClickSeries = ChartsClickSeries
    BottomAxis.ExactDateTime = False
    BottomAxis.Increment = 0.001000000000000000
    BottomAxis.Title.Caption = #1074#1088#1077#1084#1103', '#1089
    LeftAxis.AxisValuesFormat = '#,##0.#'
    LeftAxis.ExactDateTime = False
    LeftAxis.Increment = 0.100000000000000000
    LeftAxis.Title.Caption = #1085#1072#1087#1088#1103#1078#1077#1085#1080#1077', '#1084#1042
    Legend.Visible = False
    RightAxis.Visible = False
    TopAxis.Labels = False
    TopAxis.Visible = False
    View3D = False
    OnAfterDraw = AmplInTAfterDraw
    TabOrder = 1
    Visible = False
    OnDblClick = GraphChartDblClick
    object gAmplsL: TLineSeries
      Marks.Arrow.Mode = pmMerge
      Marks.ArrowLength = 8
      Marks.Clip = True
      Marks.Frame.Mode = pmNop
      Marks.Frame.SmallDots = True
      Marks.Visible = False
      SeriesColor = clBlack
      Title = 'gAmplsL'
      Dark3D = False
      LinePen.Color = clRed
      Pointer.Brush.Color = clRed
      Pointer.HorizSize = 5
      Pointer.InflateMargins = True
      Pointer.Style = psCircle
      Pointer.VertSize = 5
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
    object curAmpl: TPointSeries
      Marks.ArrowLength = 0
      Marks.Clip = True
      Marks.Style = smsLegend
      Marks.Visible = False
      SeriesColor = clYellow
      Title = 'curAmpl'
      Pointer.InflateMargins = True
      Pointer.Style = psRectangle
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
  end
  object RiseTInT: TChart
    Tag = 2
    Left = 8
    Top = 231
    Width = 150
    Height = 113
    AllowPanning = pmNone
    BackWall.Brush.Color = clWhite
    BackWall.Brush.Style = bsClear
    Title.Text.Strings = (
      #1042#1088#1077#1084#1103' '#1085#1072#1088#1072#1089#1090#1072#1085#1080#1103)
    OnClickSeries = ChartsClickSeries
    BottomAxis.ExactDateTime = False
    BottomAxis.Increment = 0.001000000000000000
    BottomAxis.Title.Caption = #1074#1088#1077#1084#1103', '#1089
    LeftAxis.ExactDateTime = False
    LeftAxis.Increment = 1.000000000000000000
    LeftAxis.Title.Caption = #1074#1088#1077#1084#1103', '#1084#1082#1089
    Legend.Visible = False
    RightAxis.Visible = False
    TopAxis.Labels = False
    TopAxis.Visible = False
    View3D = False
    TabOrder = 2
    Visible = False
    OnDblClick = GraphChartDblClick
    object gRiseTsL: TLineSeries
      Marks.ArrowLength = 8
      Marks.Visible = False
      SeriesColor = clBlack
      Title = 'gRiseTsL'
      Dark3D = False
      Pointer.Brush.Color = clRed
      Pointer.HorizSize = 5
      Pointer.InflateMargins = True
      Pointer.Style = psCircle
      Pointer.VertSize = 5
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
    object curRiseT: TPointSeries
      Marks.ArrowLength = 0
      Marks.Visible = False
      SeriesColor = clYellow
      Title = 'curRiseT'
      Pointer.InflateMargins = True
      Pointer.Style = psRectangle
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
  end
  object FallTInT: TChart
    Tag = 3
    Left = 87
    Top = 175
    Width = 144
    Height = 113
    AllowPanning = pmNone
    BackWall.Brush.Color = clWhite
    BackWall.Brush.Style = bsClear
    Title.Text.Strings = (
      #1042#1088#1077#1084#1103' '#1089#1087#1072#1076#1072)
    OnClickSeries = ChartsClickSeries
    BottomAxis.ExactDateTime = False
    BottomAxis.Increment = 0.001000000000000000
    BottomAxis.Title.Caption = #1074#1088#1077#1084#1103', '#1089
    LeftAxis.ExactDateTime = False
    LeftAxis.Increment = 1.000000000000000000
    LeftAxis.Title.Caption = #1074#1088#1077#1084#1103', '#1084#1082#1089
    Legend.Visible = False
    RightAxis.Visible = False
    TopAxis.Labels = False
    TopAxis.Visible = False
    View3D = False
    TabOrder = 3
    Visible = False
    OnDblClick = GraphChartDblClick
    object gFallTsL: TLineSeries
      Marks.ArrowLength = 8
      Marks.Visible = False
      SeriesColor = clBlack
      Title = 'gFallTsL'
      Dark3D = False
      Pointer.Brush.Color = clRed
      Pointer.HorizSize = 5
      Pointer.InflateMargins = True
      Pointer.Style = psCircle
      Pointer.VertSize = 5
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
    object curFallT: TPointSeries
      Marks.ArrowLength = 0
      Marks.Visible = False
      SeriesColor = clYellow
      Title = 'curFallT'
      Pointer.InflateMargins = True
      Pointer.Style = psRectangle
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
  end
  object NaAmplInT: TChart
    Tag = 4
    Left = 290
    Top = 8
    Width = 132
    Height = 125
    AllowPanning = pmNone
    BackWall.Brush.Color = clWhite
    BackWall.Brush.Style = bsClear
    Title.Text.Strings = (
      'Na-'#1090#1086#1082)
    OnClickSeries = ChartsClickSeries
    BottomAxis.ExactDateTime = False
    BottomAxis.Increment = 0.001000000000000000
    BottomAxis.Title.Caption = #1074#1088#1077#1084#1103', '#1089
    LeftAxis.ExactDateTime = False
    LeftAxis.Increment = 0.100000000000000000
    LeftAxis.Title.Caption = #1085#1072#1087#1088#1103#1078#1077#1085#1080#1077', '#1084#1042
    Legend.Visible = False
    RightAxis.Visible = False
    TopAxis.Visible = False
    View3D = False
    TabOrder = 4
    Visible = False
    OnDblClick = GraphChartDblClick
    object gNaAmplsL: TLineSeries
      Marks.ArrowLength = 8
      Marks.Visible = False
      SeriesColor = clBlack
      Pointer.Brush.Color = clRed
      Pointer.HorizSize = 5
      Pointer.InflateMargins = True
      Pointer.Style = psCircle
      Pointer.VertSize = 5
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
    object curNaAmpl: TPointSeries
      Marks.ArrowLength = 0
      Marks.Visible = False
      SeriesColor = clRed
      Pointer.Brush.Color = clYellow
      Pointer.InflateMargins = True
      Pointer.Style = psRectangle
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
  end
  object NaDurInT: TChart
    Tag = 5
    Left = 217
    Top = 231
    Width = 137
    Height = 114
    AllowPanning = pmNone
    BackWall.Brush.Color = clWhite
    BackWall.Brush.Style = bsClear
    Title.Text.Strings = (
      #1076#1083#1080#1090#1077#1083#1100#1085#1086#1089#1090#1100' Na-'#1090#1086#1082#1072)
    OnClickSeries = ChartsClickSeries
    BottomAxis.Title.Caption = #1074#1088#1077#1084#1103', '#1089
    LeftAxis.ExactDateTime = False
    LeftAxis.Increment = 1.000000000000000000
    LeftAxis.Title.Caption = #1074#1088#1077#1084#1103', '#1084#1082#1089
    Legend.Visible = False
    RightAxis.Visible = False
    TopAxis.Visible = False
    View3D = False
    TabOrder = 6
    Visible = False
    OnDblClick = GraphChartDblClick
    object gNaDursL: TLineSeries
      Marks.ArrowLength = 8
      Marks.Visible = False
      SeriesColor = clBlack
      Pointer.Brush.Color = clRed
      Pointer.HorizSize = 5
      Pointer.InflateMargins = True
      Pointer.Style = psCircle
      Pointer.VertSize = 5
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
    object curNaDur: TPointSeries
      Marks.ArrowLength = 0
      Marks.Visible = False
      SeriesColor = clGreen
      Pointer.Brush.Color = clYellow
      Pointer.InflateMargins = True
      Pointer.Style = psRectangle
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
  end
  object KAmplInT: TChart
    Tag = 6
    Left = 294
    Top = 150
    Width = 128
    Height = 116
    AllowPanning = pmNone
    BackWall.Brush.Color = clWhite
    BackWall.Brush.Style = bsClear
    Title.Text.Strings = (
      'K-'#1090#1086#1082)
    OnClickSeries = ChartsClickSeries
    BottomAxis.Title.Caption = #1074#1088#1077#1084#1103', '#1089
    LeftAxis.ExactDateTime = False
    LeftAxis.Increment = 0.100000000000000000
    LeftAxis.Title.Caption = #1085#1072#1087#1088#1103#1078#1077#1085#1080#1077', '#1084#1042
    Legend.Visible = False
    RightAxis.Visible = False
    TopAxis.Visible = False
    View3D = False
    TabOrder = 7
    Visible = False
    OnDblClick = GraphChartDblClick
    object gKAmplsL: TLineSeries
      Marks.ArrowLength = 8
      Marks.Visible = False
      SeriesColor = clBlack
      Pointer.Brush.Color = clRed
      Pointer.HorizSize = 5
      Pointer.InflateMargins = True
      Pointer.Style = psCircle
      Pointer.VertSize = 5
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
    object curKAmpl: TPointSeries
      Marks.ArrowLength = 0
      Marks.Visible = False
      SeriesColor = clGreen
      Pointer.Brush.Color = clYellow
      Pointer.InflateMargins = True
      Pointer.Style = psRectangle
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
  end
  object SynDelInT: TChart
    Tag = 7
    Left = 374
    Top = 199
    Width = 145
    Height = 129
    AllowPanning = pmNone
    BackWall.Brush.Color = clWhite
    BackWall.Brush.Style = bsClear
    Title.Text.Strings = (
      #1089#1080#1085#1072#1087#1090#1080#1095#1077#1089#1082#1072#1103' '#1079#1072#1076#1077#1088#1078#1082#1072)
    OnClickSeries = ChartsClickSeries
    BottomAxis.ExactDateTime = False
    BottomAxis.Increment = 0.001000000000000000
    BottomAxis.Title.Caption = #1074#1088#1077#1084#1103', '#1089
    DepthAxis.Automatic = False
    DepthAxis.AutomaticMaximum = False
    DepthAxis.AutomaticMinimum = False
    DepthAxis.Maximum = 0.500000000000000000
    DepthAxis.Minimum = -0.500000000000000000
    LeftAxis.ExactDateTime = False
    LeftAxis.Increment = 0.100000000000000000
    LeftAxis.Title.Caption = #1074#1088#1077#1084#1103', '#1084#1082#1089
    Legend.Visible = False
    RightAxis.Automatic = False
    RightAxis.AutomaticMaximum = False
    RightAxis.AutomaticMinimum = False
    RightAxis.Visible = False
    TopAxis.Visible = False
    View3D = False
    TabOrder = 8
    Visible = False
    OnDblClick = GraphChartDblClick
    object gSynDelsL: TLineSeries
      Marks.ArrowLength = 8
      Marks.Style = smsXValue
      Marks.Visible = False
      SeriesColor = clBlack
      Pointer.Brush.Color = clRed
      Pointer.HorizSize = 5
      Pointer.InflateMargins = True
      Pointer.Style = psCircle
      Pointer.VertSize = 5
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
    object curSynDel: TPointSeries
      Marks.ArrowLength = 0
      Marks.Visible = False
      SeriesColor = clGreen
      Pointer.Brush.Color = clYellow
      Pointer.InflateMargins = True
      Pointer.Style = psRectangle
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
  end
  object SynTrnsInT: TChart
    Tag = 8
    Left = 400
    Top = 67
    Width = 119
    Height = 112
    AllowPanning = pmNone
    BackWall.Brush.Color = clWhite
    BackWall.Brush.Style = bsClear
    Title.Text.Strings = (
      #1089#1080#1085#1072#1087#1090' '#1087#1088#1074#1076)
    OnClickSeries = ChartsClickSeries
    BottomAxis.ExactDateTime = False
    BottomAxis.Increment = 1.000000000000000000
    BottomAxis.Title.Caption = #1074#1088#1077#1084#1103', '#1089
    LeftAxis.ExactDateTime = False
    LeftAxis.Increment = 1.000000000000000000
    LeftAxis.Title.Caption = #1074#1088#1077#1084#1103', '#1084#1082#1089
    Legend.Visible = False
    RightAxis.Visible = False
    TopAxis.Visible = False
    View3D = False
    TabOrder = 9
    Visible = False
    OnDblClick = GraphChartDblClick
    object gSynTrnsL: TLineSeries
      Marks.ArrowLength = 8
      Marks.Visible = False
      SeriesColor = clBlack
      Pointer.Brush.Color = clRed
      Pointer.HorizSize = 5
      Pointer.InflateMargins = True
      Pointer.Style = psCircle
      Pointer.VertSize = 5
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
    object curSynTrns: TPointSeries
      Marks.ArrowLength = 0
      Marks.Visible = False
      SeriesColor = clGreen
      Pointer.Brush.Color = clYellow
      Pointer.InflateMargins = True
      Pointer.Style = psRectangle
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
  end
  object pValues: TStringGrid
    Left = 105
    Top = 323
    Width = 170
    Height = 81
    ColCount = 18
    DefaultColWidth = 20
    RowCount = 2
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goColSizing, goTabs, goThumbTracking]
    PopupMenu = PopupMenu1
    TabOrder = 5
    Visible = False
    OnSelectCell = pValuesSelectCell
    ColWidths = (
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20)
  end
  object MainMenu1: TMainMenu
    object gphFile: TMenuItem
      Caption = #1060#1072#1081#1083
      object gphOpen: TMenuItem
        Caption = #1054#1090#1082#1088#1099#1090#1100
        ShortCut = 16463
        OnClick = gphOpenClick
      end
      object gphSave: TMenuItem
        Caption = #1057#1086#1093#1088#1072#1085#1080#1090#1100' '#1082#1072#1082
        Enabled = False
        ShortCut = 16467
        OnClick = gphSaveClick
      end
      object parametrExport: TMenuItem
        Caption = #1069#1082#1089#1087#1086#1088#1090' '#1087#1072#1088#1072#1084#1077#1090#1088#1086#1074
        Enabled = False
        ShortCut = 16453
        OnClick = parametrExportClick
      end
      object signalsExport: TMenuItem
        Caption = #1069#1082#1089#1087#1086#1088#1090' '#1089#1080#1075#1085#1072#1083#1086#1074
        Enabled = False
        ShortCut = 16466
        OnClick = signalsExportClick
      end
      object prntWin: TMenuItem
        Caption = #1087#1077#1095#1072#1090#1100
        Enabled = False
        ShortCut = 16464
        OnClick = PrintWinClick
      end
    end
    object signalManager: TMenuItem
      Caption = #1057#1080#1075#1085#1072#1083#1099
      ShortCut = 16449
      Visible = False
      object prevSignal: TMenuItem
        Caption = #1087#1088#1077#1076#1099#1076#1091#1097#1080#1081' '#1089#1080#1075#1085#1072#1083
        ShortCut = 16452
        OnClick = PrevPres
      end
      object nextSignal: TMenuItem
        Caption = #1089#1083#1077#1076#1091#1102#1097#1080#1081' '#1089#1080#1075#1085#1072#1083
        ShortCut = 16454
        OnClick = NextPres
      end
      object deletSignal: TMenuItem
        Caption = #1091#1076#1072#1083#1080#1090#1100' '#1089#1080#1075#1085#1072#1083
        ShortCut = 46
        OnClick = DeletSignalClick
      end
      object remGrpSigs: TMenuItem
        Caption = #1091#1076#1072#1083#1080#1090#1100' '#1075#1088#1091#1087#1087#1091
        ShortCut = 16430
        OnClick = remGrpSigsClick
      end
      object saveGrpSigs: TMenuItem
        Caption = #1089#1086#1093#1088#1072#1085#1080#1090#1100' '#1075#1088#1091#1087#1087#1091
        ShortCut = 49235
        OnClick = saveGrpSigsClick
      end
      object dividGrps: TMenuItem
        Caption = #1088#1072#1079#1073#1080#1090#1100' '#1085#1072' '#1075#1088#1091#1087#1087#1099
        OnClick = dividGrpsClick
      end
    end
    object ShowMean: TMenuItem
      Caption = #1057#1088#1077#1076#1085#1080#1081
      Visible = False
      OnClick = ShowMeanClick
    end
    object SetExpAs: TMenuItem
      Caption = #1057#1084#1077#1085#1080#1090#1100
      Visible = False
      object asIntracel: TMenuItem
        Caption = #1085#1072' '#1042#1053#1059#1058#1056#1048#1082#1083#1077#1090
        OnClick = asIntracelClick
      end
      object asExtracel: TMenuItem
        Caption = #1085#1072' '#1042#1053#1045#1082#1083#1077#1090
        OnClick = asExtracelClick
      end
      object asMini: TMenuItem
        Caption = #1085#1072' '#1052#1080#1085#1080#1072#1090#1102#1088#1085
        OnClick = asMiniClick
      end
    end
    object expInfo: TMenuItem
      Caption = #1055#1086#1082#1072#1079#1072#1090#1100
      Visible = False
      object usersInfo: TMenuItem
        Caption = #1041#1083#1086#1082#1085#1086#1090
        OnClick = usersInfoClick
      end
      object expParams: TMenuItem
        Caption = #1044#1072#1085#1085#1099#1077' '#1086' '#1092#1072#1081#1083#1077
        OnClick = expParamsClick
      end
      object showMarks: TMenuItem
        Caption = #1047#1072#1084#1077#1090#1082#1080' '#1087#1086' '#1093#1086#1076#1091
        Checked = True
        OnClick = showMarksClick
      end
    end
    object ReCalculat: TMenuItem
      Caption = #1055#1077#1088#1077#1089#1095#1105#1090
      Visible = False
      OnClick = ReCalculatClick
    end
    object ShowChanN: TMenuItem
      Caption = #1042#1089#1077' '#1082#1072#1085#1072#1083#1099
      Visible = False
      object SAllChan: TMenuItem
        Caption = #1074#1089#1077' '#1082#1072#1085#1072#1083#1099
        Checked = True
        RadioItem = True
        OnClick = SAllChanClick
      end
    end
  end
  object OpenDlg: TOpenDialog
    Filter = '*.pra|*.pra|*.drr|*.drr|*.bin|*.bin'
    Left = 37
  end
  object SaveDlg: TSaveDialog
    DefaultExt = 'pra'
    Filter = '*.pra|*.pra'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofEnableSizing]
    Title = #1042#1074#1077#1076#1080#1090#1077' '#1080#1084#1103' '#1092#1072#1081#1083#1072
    Left = 72
  end
  object PopupMenu1: TPopupMenu
    Left = 207
    Top = 353
    object selCopy: TMenuItem
      Caption = 'Copy'
      ShortCut = 16451
      OnClick = selCopyClick
    end
    object selAll: TMenuItem
      Caption = 'Select All'
      ShortCut = 16449
      OnClick = selAllClick
    end
  end
end
