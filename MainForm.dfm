object Experiment: TExperiment
  Left = 5
  Top = 0
  Anchors = [akLeft, akTop, akRight, akBottom]
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Elph'
  ClientHeight = 250
  ClientWidth = 790
  Color = clBtnFace
  DefaultMonitor = dmMainForm
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  Position = poDesigned
  OnActivate = FormActivate
  OnCloseQuery = FormCloseQuery
  OnDestroy = ReleasMem
  PixelsPerInch = 96
  TextHeight = 13
  object deviceBox: TGroupBox
    Left = 594
    Top = -1
    Width = 195
    Height = 160
    Caption = #1051#1040'-2USB  ('#1076#1088#1072#1081#1074#1077#1088' LA2USB)'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 0
    object Label2: TLabel
      Left = 123
      Top = 11
      Width = 62
      Height = 11
      Caption = #1073#1072#1079#1086#1074#1099#1081' '#1072#1076#1088#1077#1089
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label3: TLabel
      Left = 5
      Top = 57
      Width = 77
      Height = 13
      Caption = #1076#1080#1089#1082#1088#1077#1090#1080#1079#1072#1094#1080#1103
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label21: TLabel
      Left = 19
      Top = 14
      Width = 54
      Height = 14
      Caption = #1091#1089#1080#1083#1077#1085#1080#1077
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label10: TLabel
      Left = 63
      Top = 77
      Width = 17
      Height = 13
      Caption = #1084#1082#1089
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label24: TLabel
      Left = 98
      Top = 48
      Width = 90
      Height = 13
      Caption = #1095#1080#1089#1083#1086'     '#1074#1077#1076#1091#1097#1080#1081
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object mChnlRecModLbl: TLabel
      Left = 100
      Top = 100
      Width = 85
      Height = 13
      Caption = #1084#1085#1086#1075#1086#1082#1072#1085#1072#1083#1100#1085#1099#1081
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object DiscntWrtLbl: TLabel
      Left = 12
      Top = 100
      Width = 73
      Height = 13
      Caption = #1087#1088#1103#1084#1072#1103' '#1079#1072#1087#1080#1089#1100
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label8: TLabel
      Left = 96
      Top = 60
      Width = 84
      Height = 13
      Caption = #1082#1072#1085#1072#1083#1086#1074'    '#1082#1072#1085#1072#1083
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object uiBase: TComboBox
      Left = 122
      Top = 23
      Width = 64
      Height = 21
      Style = csDropDownList
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ItemHeight = 13
      ParentFont = False
      TabOrder = 0
      OnChange = uiBaseChange
    end
    object DevEvents: TEdit
      Left = 4
      Top = 133
      Width = 185
      Height = 22
      AutoSize = False
      BiDiMode = bdRightToLeft
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentBiDiMode = False
      ParentFont = False
      ReadOnly = True
      TabOrder = 1
      Text = 'OK'
    end
    object uiChanAct: TComboBox
      Left = 93
      Top = 74
      Width = 45
      Height = 21
      Style = csDropDownList
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ItemHeight = 13
      ParentFont = False
      TabOrder = 2
      OnChange = uiChanActChange
    end
    object uiGain: TComboBox
      Left = 15
      Top = 29
      Width = 62
      Height = 21
      Style = csDropDownList
      BiDiMode = bdRightToLeftReadingOnly
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ItemHeight = 13
      ParentBiDiMode = False
      ParentFont = False
      TabOrder = 3
      OnChange = uiGainChange
    end
    object DiscreTime: TEdit
      Left = 9
      Top = 74
      Width = 50
      Height = 21
      BiDiMode = bdRightToLeft
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentBiDiMode = False
      ParentFont = False
      TabOrder = 4
      OnChange = DiscreTimeChange
      OnKeyPress = CheckForKeyOtherEdit
    end
    object RecMode: TCheckBox
      Left = 116
      Top = 114
      Width = 56
      Height = 15
      Caption = #1088#1077#1078#1080#1084
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 5
      OnClick = RecModeClick
    end
    object DiscontinWrt: TCheckBox
      Left = 17
      Top = 114
      Width = 63
      Height = 15
      Caption = #1085#1072' '#1076#1080#1089#1082
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 6
      OnClick = DiscontinWrtClick
    end
    object uiLeadChan: TComboBox
      Left = 142
      Top = 74
      Width = 45
      Height = 21
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ItemHeight = 13
      ItemIndex = 0
      ParentFont = False
      TabOrder = 7
      Text = '1'
      OnChange = uiLeadChanChange
      Items.Strings = (
        '1')
    end
  end
  object exptBox: TGroupBox
    Left = 2
    Top = -1
    Width = 174
    Height = 160
    Caption = #1042#1099#1087#1086#1083#1085#1080#1090#1100
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 1
    object InducIntracell: TButton
      Left = 32
      Top = 14
      Width = 103
      Height = 25
      Caption = #1042#1053#1059#1058#1056#1048#1082#1083#1077#1090#1086#1095#1085#1099#1081
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      OnClick = InducIntracellClick
    end
    object SpontanSignal: TButton
      Left = 97
      Top = 45
      Width = 71
      Height = 25
      Caption = #1057#1087#1086#1085#1090#1072#1085#1085#1099#1081
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
      OnClick = SpontanSignalClick
    end
    object ShowOscil: TButton
      Left = 47
      Top = 117
      Width = 80
      Height = 32
      Caption = #1054#1089#1094#1080#1083#1083#1086#1075#1088#1072#1092
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 2
      OnClick = ShowOscilClick
    end
    object InducExtracell: TButton
      Left = 6
      Top = 45
      Width = 81
      Height = 25
      Caption = #1042#1053#1045#1082#1083#1077#1090#1086#1095#1085#1099#1081
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 3
      OnClick = InducExtracellClick
    end
    object InducSspontS: TButton
      Left = 26
      Top = 76
      Width = 109
      Height = 25
      Caption = #1042#1099#1079#1074#1072#1085#1085#1099#1081'+'#1057#1087#1085#1090
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 4
      OnClick = InducSspontSClick
    end
  end
  object expSet: TGroupBox
    Left = 179
    Top = -1
    Width = 412
    Height = 160
    Caption = #1053#1072#1089#1090#1088#1086#1081#1082#1080' '#1090#1077#1082#1091#1097#1077#1075#1086' '#1101#1082#1089#1087#1077#1088#1080#1084#1077#1085#1090#1072
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 2
    object stimLbl2: TLabel
      Left = 12
      Top = 29
      Width = 141
      Height = 13
      Caption = #1095#1072#1089#1090#1086#1090#1072'              '#1080#1083#1080' '#1087#1077#1088#1080#1086#1076
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object stimLbl3: TLabel
      Left = 75
      Top = 48
      Width = 12
      Height = 13
      Caption = #1043#1094
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object stimLbl4: TLabel
      Left = 161
      Top = 48
      Width = 11
      Height = 13
      Caption = #1084#1089
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object nsLbl1: TLabel
      Left = 181
      Top = 18
      Width = 59
      Height = 13
      Caption = #1082#1086#1083#1080#1095#1077#1089#1090#1074#1086
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object nsLbl2: TLabel
      Left = 187
      Top = 29
      Width = 46
      Height = 13
      Caption = #1089#1080#1075#1085#1072#1083#1086#1074
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object stimLbl1: TLabel
      Left = 43
      Top = 18
      Width = 59
      Height = 13
      Caption = #1089#1090#1080#1084#1091#1083#1103#1094#1080#1103
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object StimFreq: TEdit
      Left = 6
      Top = 45
      Width = 63
      Height = 21
      BiDiMode = bdRightToLeft
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ImeMode = imClose
      ParentBiDiMode = False
      ParentFont = False
      TabOrder = 0
      Text = '20'
      OnChange = StimFreqChange
      OnKeyPress = CheckForKeyStimFreqEdit
    end
    object StimPeriod: TEdit
      Left = 99
      Top = 45
      Width = 55
      Height = 21
      BiDiMode = bdRightToLeft
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ImeMode = imClose
      ParentBiDiMode = False
      ParentFont = False
      TabOrder = 1
      Text = '50'
      OnChange = StimPeriodChange
      OnKeyPress = CheckForKeyOtherEdit
    end
    object NumOfsignals: TEdit
      Left = 181
      Top = 45
      Width = 59
      Height = 21
      BiDiMode = bdRightToLeft
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentBiDiMode = False
      ParentFont = False
      TabOrder = 2
      Text = '10'
      OnChange = NumOfsignalsChange
      OnKeyPress = CheckForKeyOtherEdit
    end
    object SaveTo: TButton
      Left = 6
      Top = 109
      Width = 75
      Height = 25
      Caption = #1089#1086#1093#1088#1072#1085#1080#1090#1100' '#1074
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 3
      OnClick = SaveToClick
    end
    object FilePathName: TEdit
      Left = 6
      Top = 133
      Width = 400
      Height = 22
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
      TabOrder = 4
      Text = 'c:\Signal.pra'
    end
    object SetScenar: TButton
      Left = 6
      Top = 74
      Width = 127
      Height = 29
      Caption = #1047#1072#1076#1072#1090#1100' '#1089#1090#1080#1084#1091#1083#1103#1094#1080#1102
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 5
      OnClick = SetScenarClick
    end
  end
  object visualization: TGroupBox
    Left = 2
    Top = 160
    Width = 787
    Height = 88
    Caption = #1043#1088#1072#1092#1080#1082#1080' '#1080' '#1075#1080#1089#1090#1086#1075#1088#1072#1084#1084#1099
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 3
    object Label1: TLabel
      Left = 503
      Top = 17
      Width = 210
      Height = 13
      Caption = #1043#1080#1089#1090#1086#1075#1088#1072#1084#1084#1099' '#1088#1072#1089#1087#1088#1077#1076#1077#1083#1077#1085#1080#1103' '#1087#1072#1088#1072#1084#1077#1090#1088#1086#1074
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label20: TLabel
      Left = 52
      Top = 18
      Width = 173
      Height = 13
      Caption = #1042#1088#1077#1084#1077#1085#1085#1072#1103' '#1088#1072#1079#1074#1105#1088#1090#1082#1072' '#1087#1072#1088#1072#1084#1077#1090#1088#1086#1074
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label12: TLabel
      Left = 320
      Top = 64
      Width = 41
      Height = 13
      Caption = #1082#1072#1078#1076#1099#1077
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label13: TLabel
      Left = 420
      Top = 64
      Width = 46
      Height = 13
      Caption = #1089#1080#1075#1085#1072#1083#1086#1074
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Ampl_gist: TButton
      Tag = 1
      Left = 485
      Top = 36
      Width = 70
      Height = 21
      Caption = #1072#1084#1087#1083#1080#1090#1091#1076#1072
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      OnClick = HistogramClick
    end
    object RiseTime_gist: TButton
      Tag = 2
      Left = 561
      Top = 36
      Width = 95
      Height = 21
      Caption = #1074#1088' '#1085#1072#1088#1072#1089#1090#1072#1085#1080#1103
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
      OnClick = HistogramClick
    end
    object FallTime_gist: TButton
      Tag = 3
      Left = 663
      Top = 36
      Width = 60
      Height = 21
      Caption = #1074#1088' '#1089#1087#1072#1076#1072
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 2
      OnClick = HistogramClick
    end
    object Ampl_inT: TButton
      Tag = 1
      Left = 4
      Top = 36
      Width = 70
      Height = 21
      Caption = #1072#1084#1087#1083#1080#1090#1091#1076#1072
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 3
      OnClick = ParameterClick
    end
    object Rise_inT: TButton
      Tag = 2
      Left = 78
      Top = 36
      Width = 95
      Height = 21
      Caption = #1074#1088' '#1085#1072#1088#1072#1089#1090#1072#1085#1080#1103
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 4
      OnClick = ParameterClick
    end
    object Fall_inT: TButton
      Tag = 3
      Left = 178
      Top = 36
      Width = 60
      Height = 21
      Caption = #1074#1088' '#1089#1087#1072#1076#1072
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 5
      OnClick = ParameterClick
    end
    object ggRefresh: TCheckBox
      Left = 335
      Top = 40
      Width = 119
      Height = 17
      Caption = #1086#1073#1085#1086#1074#1083#1103#1090#1100' '#1075#1088#1072#1092#1080#1082#1080
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 6
      OnClick = ggRefreshClick
    end
    object refreshEvery: TEdit
      Left = 365
      Top = 60
      Width = 50
      Height = 21
      BiDiMode = bdRightToLeft
      Enabled = False
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentBiDiMode = False
      ParentFont = False
      TabOrder = 7
      Text = '10'
      OnChange = refreshEveryChange
    end
    object RateMS_inTime: TButton
      Left = 728
      Top = 9
      Width = 54
      Height = 22
      Caption = #1095#1072#1089#1090#1086#1090#1072
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 8
      OnClick = HistogramClick
    end
    object NaAmpl_inT: TButton
      Tag = 4
      Left = 4
      Top = 62
      Width = 64
      Height = 21
      Caption = 'Na-'#1090#1086#1082
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 9
      OnClick = ParameterClick
    end
    object TabsCheck: TCheckBox
      Left = 323
      Top = 12
      Width = 146
      Height = 16
      Hint = #1077#1089#1083#1080' true, '#1074#1099#1074#1086#1076#1080#1084' '#1090#1072#1073#1083#1080#1094#1099
      Caption = #1090#1072#1073#1083#1080#1094#1072' '#1087#1072#1088#1072#1084#1077#1090#1088#1086#1074
      TabOrder = 10
      OnClick = TabsCheckClick
    end
    object KAmpl_inT: TButton
      Tag = 6
      Left = 149
      Top = 62
      Width = 70
      Height = 21
      Caption = 'K-'#1090#1086#1082
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 11
      OnClick = ParameterClick
    end
    object St0_inT: TButton
      Tag = 8
      Left = 243
      Top = 36
      Width = 54
      Height = 21
      Caption = 'st0'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 12
      OnClick = ParameterClick
    end
    object SynDel_inT: TButton
      Tag = 7
      Left = 226
      Top = 62
      Width = 71
      Height = 21
      Caption = #1089#1080#1085#1072#1087#1090' '#1079
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 13
      OnClick = ParameterClick
    end
    object NaDur_inT: TButton
      Tag = 5
      Left = 76
      Top = 62
      Width = 66
      Height = 21
      Caption = #1076#1083#1080#1090' Na'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 14
      OnClick = ParameterClick
    end
    object NaAmpl_gist: TButton
      Tag = 4
      Left = 485
      Top = 62
      Width = 64
      Height = 21
      Caption = 'Na-'#1090#1086#1082
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 15
      OnClick = HistogramClick
    end
    object NaDur_gist: TButton
      Tag = 5
      Left = 558
      Top = 62
      Width = 66
      Height = 21
      Caption = #1076#1083#1080#1090' Na'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 16
      OnClick = HistogramClick
    end
    object St0_gist: TButton
      Tag = 8
      Left = 728
      Top = 36
      Width = 54
      Height = 21
      Caption = 'st0'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 17
      OnClick = HistogramClick
    end
    object KAmpl_gist: TButton
      Tag = 6
      Left = 634
      Top = 62
      Width = 70
      Height = 21
      Caption = 'K-'#1090#1086#1082
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 18
      OnClick = HistogramClick
    end
    object SynDel_gist: TButton
      Tag = 7
      Left = 711
      Top = 62
      Width = 71
      Height = 21
      Caption = #1089#1080#1085#1072#1087#1090' '#1079
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 19
      OnClick = HistogramClick
    end
  end
  object MainMenu1: TMainMenu
    Left = 332
    Top = 89
    object beginFOpen: TMenuItem
      Caption = #1054#1090#1082#1088#1099#1090#1100' '#1092#1072#1081#1083
      ShortCut = 16463
      OnClick = beginFOpenClick
    end
    object verticalScal: TMenuItem
      Caption = 'Y_'#1042#1077#1088#1090#1080#1082#1072#1083#1100#1085'. '#1096#1082#1072#1083#1072
      object shwAmpls: TMenuItem
        Tag = 1
        Caption = #1072#1084#1087#1083#1080#1090#1091#1076#1099
        Checked = True
        RadioItem = True
        OnClick = VerticAxisMode
      end
      object pCentFirstS: TMenuItem
        Caption = '% '#1087#1077#1088#1074#1086#1075#1086
        RadioItem = True
        OnClick = VerticAxisMode
      end
      object pCentMax: TMenuItem
        Caption = '% '#1084#1072#1082#1089#1080#1084#1091#1084#1072
        RadioItem = True
        OnClick = VerticAxisMode
      end
    end
    object timScal: TMenuItem
      Caption = 'X_'#1043#1086#1088#1080#1079#1086#1085#1090#1072#1083#1100#1085'. '#1096#1082#1072#1083#1072
      object inSeconds: TMenuItem
        Caption = #1074' '#1089#1077#1082#1091#1085#1076#1072#1093
        Checked = True
        RadioItem = True
        OnClick = HorizontAxisMode
      end
      object inMinutes: TMenuItem
        Tag = 1
        Caption = #1074' '#1084#1080#1085#1091#1090#1072#1093
        RadioItem = True
        OnClick = HorizontAxisMode
      end
      object inSignalNum: TMenuItem
        Tag = 2
        Caption = #1085#1086#1084#1077#1088#1072' '#1089#1080#1075#1085#1072#1083#1086#1074
        Enabled = False
        RadioItem = True
        OnClick = HorizontAxisMode
      end
    end
  end
end
