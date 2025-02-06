import 'package:fluent_ui/fluent_ui.dart';

class WindowsHomePage extends StatelessWidget {
  const WindowsHomePage({super.key});

  @override
  Widget build(BuildContext context) {
    return ScaffoldPage(
      header: PageHeader(title: Text('Windows App')),
      content: Center(child: Text('Welcome to Windows UI')),
    );
  }
}
